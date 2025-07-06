#include <iostream>
#include "wpa_ctrl.h"
#include <cstring>
#include <regex>
#include <thread>
#include <vector>
#include <optional>
#include <fstream>
#include <filesystem>
#include "wifi_helper.h"
#include <unistd.h>
#include <curl/curl.h>
#include "api.h"
#include <mutex>
#include <condition_variable>
#include "config.h"

#define DEFAULT_WIFI_INTERFACE "wlan0"

int secondsSinceLastScan = 0;
std::mutex mtx;
std::condition_variable cv;
bool scanComplete = false;

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
    std::cout << "Receive event: " << event << std::endl;
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

void run_wifi_task(WifiHelper &wifi, const ConfigData &config)
{
    std::string state;
    if (!wifi.get_wifi_state(state))
    {
        std::cout << "failed to get wifi state" << std::endl;
        return;
    }

    if (secondsSinceLastScan >= config.interval)
    {
        // scan for hotspots
        secondsSinceLastScan = 0;
        wifi.start_scanning();

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []
                { return scanComplete; });
        scanComplete = false;

        std::vector<WifiNetwork> results;
        if (!wifi.get_scan_results(results))
        {
            std::cout << "failed to get scan results" << std::endl;
            return;
        }

        if (state == "DISCONNECTED" || state == "INACTIVE")
        {
            //    Look for XFINITY hotspot and connect
            wifi.try_connect_hotspot(results);
        }
        else
        {
            // Send to the server
            API::sendNetworks(config.serverUrl, results);
        }
    }

    std::cout << "[DEBUG] Wifi state: " << state << std::endl;
    secondsSinceLastScan++;
}

std::string getDirectory()
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
    std::filesystem::path binaryFile(getDirectory());
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
        if (wifiHelper.init_events(config.interface, handleEvent) && wifiHelper.init(config.interface)) {
            break;
        } else {
            std::cout << "Failed to init! Retrying in 1s..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    while (true)
    {
        run_wifi_task(wifiHelper, config);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}