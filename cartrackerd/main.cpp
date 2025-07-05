#include <iostream>
#include "wpa_ctrl.h"
#include <cstring>
#include <regex>
#include <thread>
#include <vector>
#include <optional>
#include <fstream>
#include <filesystem>

#define BUFFER_LEN 4096
#define SCAN_INTERVAL_SEC 5
#define DEFAULT_WIFI_INTERFACE "wlan0"

bool didScan = false;
int secondsSinceLastScan = 0;
bool isConnected = false;

struct WifiNetwork
{
    std::string bssid;
    int frequency;
    int rssid;
    std::string flags;
    std::optional<std::string> ssid;
};

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

int send_request(wpa_ctrl *interface, std::string request, std::string &response, void (*msg_cb)(char *msg, size_t len))
{
    std::string buffer(BUFFER_LEN, '\0');
    size_t reply_len = buffer.size();

    int result = wpa_ctrl_request(interface, request.c_str(), request.size(), &buffer[0], &reply_len, msg_cb);

    if (result != 0)
    {
        return result;
    }

    buffer.resize(reply_len);

    response = buffer;
    return result;
}

bool get_wifi_state(wpa_ctrl *ctrl, std::string &out)
{
    std::string res;
    if (send_request(ctrl, "STATUS", res, nullptr) != 0)
    {
        return false;
    }

    std::regex exp("wpa_state=(.*)");
    std::smatch matches;
    std::regex_search(res, matches, exp);

    if (matches.length() < 2)
    {
        return false;
    }

    out = matches[1].str();
    return true;
}

bool start_scanning(wpa_ctrl *ctrl)
{
    std::string res;

    if (send_request(ctrl, "SCAN", res, [](char *msg, size_t len)
                     { std::cout << "we got a callback!" << std::endl; }) == -1)
    {
        return false;
    }

    return true;
}

bool get_scan_results(wpa_ctrl *ctrl, std::vector<WifiNetwork> &results)
{
    std::string res;

    if (send_request(ctrl, "SCAN_RESULTS", res, nullptr) == -1)
    {
        return false;
    }

    std::regex scanRegex("^([a-fA-F0-9:]{17})\\t(\\d+)\\t(\\d+)\\t([^\\t]*)\\t?([^\\t\\n]*)$");

    std::stringstream resStream(res);
    std::string line;

    std::vector<WifiNetwork> wifis;

    while (std::getline(resStream, line, '\n'))
    {
        try
        {
            std::smatch matches;
            std::regex_search(line, matches, scanRegex);
            // bssid / frequency / signal level / flags / ssid

            if (matches.length() < 5)
            {
                continue;
            }

            std::string bssid = matches[1].str();
            int frequency = std::stoi(matches[2].str());
            int signalLevel = std::stoi(matches[3].str());
            std::string flags = matches[4].str();

            std::optional<std::string> ssid;
            if (matches.length() >= 4)
            {
                ssid = matches[5].str();
            }

            WifiNetwork wifi{
                bssid, frequency, signalLevel, flags, ssid};

            wifis.push_back(wifi);
        }
        catch (std::exception &ex)
        {
            std::cout << "Failed to parse line: " << ex.what();
        }
    }

    results = wifis;

    return true;
}

void try_connect_hotspot(wpa_ctrl *ctrl, std::vector<WifiNetwork> networks)
{
    for (WifiNetwork network : networks)
    {
        if (!network.ssid.has_value())
            continue;
        if (network.ssid.value() == "XFINITY")
        {
            std::cout << "FOUND XFINITY" << std::endl;
        }
    }
}

void run_wifi_task(wpa_ctrl *ctrl)
{
    std::string state;
    if (!get_wifi_state(ctrl, state))
    {
        return;
    }

    if (state == "COMPLETED")
    {
        isConnected = true;
    }

    if (didScan && state != "SCANNING")
    {
        std::vector<WifiNetwork> results;
        if (!get_scan_results(ctrl, results))
        {
            std::cout << "failed to get scan results" << std::endl;
            return;
        }

        // Look for XFINITY hotspot and connect
        try_connect_hotspot(ctrl, results);

        didScan = false;
        secondsSinceLastScan = 0;
        return;
    }

    if (state == "DISCONNECTED")
    {
        isConnected = false;
        if (secondsSinceLastScan >= 5)
        {
            // scan for hotspots
            didScan = true;
            secondsSinceLastScan = 0;
            start_scanning(ctrl);
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

    std::string path("/var/run/wpa_supplicant/" + interface);
    wpa_ctrl *ctrl = wpa_ctrl_open(path.c_str());
    if (ctrl == nullptr)
    {
        std::cout << "failed to open wpa_supplicant control interface." << std::endl;
        std::cout << "please ensure the interface '" << interface << "' exists and wpa_supplicant is running!" << std::endl;
        return 1;
    }

    std::string state;
    if (!get_wifi_state(ctrl, state))
    {
        std::cout << "Failed to get wifi state" << std::endl;
        return 1;
    }

    while (true)
    {
        run_wifi_task(ctrl);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}