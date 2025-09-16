#include <iostream>
#include <wpa_ctrl.h>
#include <cstring>
#include <regex>
#include <thread>
#include <vector>
#include <optional>
#include <fstream>
#include <filesystem>
#include "wifi_helper.h"
#include <unistd.h>
#include "api.h"
#include <mutex>
#include <condition_variable>
#include "config.h"
#include "request_queue.h"
#include <signal.h>
#include "wpaservice.h"
#include "wifi_blocker.h"
#include "dhclient.h"

#define DEFAULT_WIFI_INTERFACE "wlan0"
#define MAX_SCANS_PER_INTERVAL 10

int secondsSinceLastScan = 0;
std::mutex mtx;
std::condition_variable cv;
bool scanComplete = false;
RequestQueue requestQueue;
int attemptsRemainingInInterval = MAX_SCANS_PER_INTERVAL;

std::string get_wifi_interface(std::filesystem::path programDir)
{
  std::ifstream file(programDir / "interface.txt");
  if (!file.good())
  {
    return DEFAULT_WIFI_INTERFACE;
  }

  std::string line;
  if (!std::getline(file, line))
  {
    return DEFAULT_WIFI_INTERFACE;
  }

  return line;
}

void handleEvent(std::string event)
{
  // std::cout << "Receive event: " << event << std::endl;
  if (event.contains("CTRL-EVENT-SCAN-RESULTS"))
  {
    std::cout << "SCAN RESULTS!" << std::endl;
    // Scan complete!
    {
      std::lock_guard<std::mutex> lock(mtx);
      scanComplete = true;
    }
    cv.notify_one();
  }
}

void run_wifi_task(WifiHelper &wifi, ConfigData &config)
{
  if (secondsSinceLastScan < config.interval && attemptsRemainingInInterval <= 0)
  {
    std::cout << "Waiting for scan... " << secondsSinceLastScan << "/" << config.interval << std::endl;
    ++secondsSinceLastScan;
    return;
  }

  secondsSinceLastScan = 0;

  // Delay ended, reset attempts
  if (attemptsRemainingInInterval <= 0) {
    attemptsRemainingInInterval = MAX_SCANS_PER_INTERVAL;
  }

  WifiBlocker::unblock();
  // TODO: do we need to wait after unblocking soft block?
  // std::this_thread::sleep_for(std::chrono::seconds(3));

  if (!WpaService::isRunning() && !WpaService::start(config.interface.c_str()))
  {
    std::cout << "Failed to start wpa_supplicant" << std::endl;
    return;
  }

  if (!wifi.is_ready())
  {
    if (!wifi.init(config.interface) || !wifi.init_events(config.interface, handleEvent))
    {
      std::cout << "Failed to init wpa_ctrl interface" << std::endl;
      wifi.close(config.interface);
      attemptsRemainingInInterval--;
      // secondsSinceLastScan = 0;
      return;
    }

    std::cout << "Wifi made ready?\n";
  }
  else
  {
    std::cout << "Wifi is ready?\n";
  }

  std::string state;
  if (!wifi.get_wifi_state(state))
  {
    std::cout << "failed to get wifi state" << std::endl;
    attemptsRemainingInInterval--;
    // secondsSinceLastScan = 0;
    return;
  }

  bool isRequestQueueEmpty = requestQueue.empty();

  if (state == "DISCONNECTED" || state == "INACTIVE" || isRequestQueueEmpty)
  {
    // scan for hotspots
    wifi.start_scanning();

    // wait for scan
    std::cout << "Waiting for scan to complete..." << std::endl;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(30), []
                { return scanComplete; });
    scanComplete = false;

    std::cout << "Scan complete!" << std::endl;
    std::cout << "Getting scan results..." << std::endl;

    // get scan results
    std::vector<WifiNetwork> results;
    if (!wifi.get_scan_results(results))
    {
      std::cout << "failed to get scan results" << std::endl;
      // secondsSinceLastScan = 0;
      attemptsRemainingInInterval--;
      return;
    }

    // add scan results to send queue
    requestQueue.add_request(results);

    // not connected or connecting, so connect to a hotspot if we found one
    if (state == "DISCONNECTED" || state == "INACTIVE")
    {
      if (!wifi.try_connect_hotspot(results, config.hotspotNames, config.interface))
      {
        // secondsSinceLastScan = 0;
        attemptsRemainingInInterval--;
      }
    }
  }
  else if (state == "COMPLETED")
  {
    // request a new ip
    DhClient::release(config.interface);
    DhClient::renew(config.interface);

    printf("IP requested!\n");

    if (!requestQueue.empty())
    {
      // send scans to the server
      bool shouldClose = false;
      if (API::sendNetworks(config.serverUrl, requestQueue, config))
      {
        requestQueue.clear();
        std::cout << "disconnected after send" << std::endl;
        attemptsRemainingInInterval = 0;
        shouldClose = true;
      }
      else
      {
        std::cout << "send failed!" << std::endl;
        attemptsRemainingInInterval--;
        if (attemptsRemainingInInterval == 0)
        {
          shouldClose = true;
        }
      }

      if (shouldClose)
      {
        // disconnect and block wifi card
        wifi.disconnect();
        wifi.close(config.interface);
        WpaService::stop();
        WifiBlocker::block();
      }

      return;
    }
  }

  std::cout << "[DEBUG] Wifi state: " << state << std::endl;
  secondsSinceLastScan++;
}

std::string getCurrentDirectory()
{
  char pBuf[257];
  size_t len = sizeof(pBuf) - 1;
  int bytes = readlink("/proc/self/exe", pBuf, len);
  if (bytes >= 0)
  {
    pBuf[bytes] = '\0';
  }
  else
  {
    pBuf[256] = '\0';
  }

  return std::string(pBuf);
}

int main(int argc, char **argv)
{
  signal(SIGPIPE, SIG_IGN);

  std::filesystem::path binaryFile(getCurrentDirectory());
  if (!binaryFile.has_parent_path())
  {
    std::cout << "no parent folder found, this should never happen!" << std::endl;
    std::cout << "Binary file path: " << binaryFile << std::endl;
    return 1;
  }

  std::filesystem::path programDir = binaryFile.parent_path();

  auto configPath = programDir / "config.json";

  ConfigData config;
  if (!Config::read(configPath, config))
  {
    std::cout << "failed to read config: " << configPath << std::endl;
    return 1;
  }

  std::cout << "cartrackerd by jluims" << std::endl;

  std::cout << "using interface '" << config.interface << "'" << std::endl;

  WifiHelper wifiHelper;
  while (true)
  {
    run_wifi_task(wifiHelper, config);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}