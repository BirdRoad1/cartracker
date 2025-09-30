#include <iostream>
#include <regex>
#include <thread>
#include <vector>
#include <fstream>
#include <filesystem>
#include "wifi_helper.h"
#include "api.h"
#include "config.h"
#include "request_queue.h"
#include <signal.h>
#include "dhclient.h"

#define DEFAULT_WIFI_INTERFACE "wlan0"
// #define MAX_SCANS_PER_INTERVAL 10
#define SCAN_TIMEOUT 30

int secondsSinceLastScan = 100000;
RequestQueue requestQueue;

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

void run_wifi_task(WifiHelper &wifi, ConfigData &config)
{
  if (secondsSinceLastScan < config.interval)
  {
    std::cout << "Waiting for scan... " << secondsSinceLastScan << "/" << config.interval << std::endl;
    ++secondsSinceLastScan;
    return;
  }

  secondsSinceLastScan = 0;

  std::vector<WifiNetwork> networks;
  wifi.scan(networks);
  std::cout << "Scan done!" << std::endl;
  requestQueue.add_request(networks);

  for (const WifiNetwork &network : networks)
  {
    std::cout << "Network:" << network.ssid.value_or("No ssid") << std::endl;
    std::cout << "BSSID:" << network.bssid << "\n"
              << std::endl;
    std::cout << "RSSI:" << network.rssi << "\n"
              << std::endl;
  }

  std::cout << "Connecting!" << std::endl;
  wifi.disconnect(config.interface);
  if (wifi.try_connect_hotspot(networks, config.hotspotNames, config.interface))
  {

    std::cout << "Connected. getting ip..." << std::endl;
    DhClient::renew(config.interface);

    if (API::sendNetworks(config.serverUrl, requestQueue, config))
    {
      requestQueue.clear();
      std::cout << "Sent!" << std::endl;
    }
    else
    {
      std::cout << "Failed to send!" << std::endl;
    }
    DhClient::release(config.interface);
    wifi.disconnect(config.interface);
  }
  
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