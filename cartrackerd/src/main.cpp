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

#define SCAN_INTERVAL_SEC 1
#define DEFAULT_WIFI_INTERFACE "wlan0"

bool didScan = false;
int secondsSinceLastScan = 0;
bool isConnected = false;


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

void run_wifi_task(WifiHelper& wifi)
{
    std::string state;
    if (!wifi.get_wifi_state(state))
    {
        std::cout << "failed to get wifi state" << std::endl;
        return;
    }

    if (state == "COMPLETED")
    {
        isConnected = true;
    }

    if (didScan && state != "SCANNING")
    {
        std::vector<WifiNetwork> results;
        if (!wifi.get_scan_results(results))
        {
            std::cout << "failed to get scan results" << std::endl;
            return;
        }

        // Look for XFINITY hotspot and connect
        wifi.try_connect_hotspot(results);

        didScan = false;
        secondsSinceLastScan = 0;
        return;
    }

    if (state == "DISCONNECTED" || state == "INACTIVE")
    {
        isConnected = false;
        if (secondsSinceLastScan >= SCAN_INTERVAL_SEC)
        {
            // scan for hotspots
            didScan = true;
            secondsSinceLastScan = 0;
            wifi.start_scanning();
        }
    }

    std::cout << "wifi state: " << state << std::endl;
    secondsSinceLastScan++;
}

std::string getDirectory()
{
    char pBuf[257];
    size_t len = sizeof(pBuf) - 1;
    int bytes = readlink("/proc/self/exe", pBuf, len);
    if (bytes >= 0) {
        pBuf[bytes] = '\0';
    } else {
        pBuf[256] = '\0';
    }

    return std::string(pBuf);
}

int main(int argc, char **argv)
{
    std::filesystem::path binaryFile(getDirectory());
    if (!binaryFile.has_parent_path()) {
        std::cout << "no parent folder found, this should never happen!" << std::endl;
        std::cout << "Binary file path: " << binaryFile << std::endl;
        return 1;
    }

    std::filesystem::path programDir = binaryFile.parent_path();

    std::cout << "cartrackerd by jluims" << std::endl;

    std::string interface = get_wifi_interface(programDir);
    std::cout << "using interface '" << interface << "'" << std::endl;

    WifiHelper wifiHelper;

    if (!wifiHelper.init(interface)) {
        return 0;
    }

    while (true)
    {
        run_wifi_task(wifiHelper);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}