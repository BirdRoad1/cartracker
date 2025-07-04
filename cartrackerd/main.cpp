#include <iostream>
#include "wpa_ctrl.h"
#include <cstring>
#include <regex>
#include <thread>
#include <vector>
#include <optional>

#define BUFFER_LEN 4096
#define SCAN_INTERVAL_SEC = 5

bool didScan = false;
int secondsSinceLastScan = 0;

struct WifiNetwork
{
    std::string bssid;
    int frequency;
    int rssid;
    std::string flags;
    std::optional<std::string> ssid;
};

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
                // Missing field, invalid line
                std::cout << "Ignored line: " << line << std::endl;
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

void run_wifi_task(wpa_ctrl *ctrl)
{
    std::string state;
    if (!get_wifi_state(ctrl, state))
    {
        return;
    }

    if (didScan && state != "SCANNING")
    {
        std::vector<WifiNetwork> results;
        if (!get_scan_results(ctrl, results))
        {
            std::cout << "failed to get scan results" << std::endl;
            return;
        }

        // Send the wifi networks to our server now
        int a = 1;

        didScan = false;
        secondsSinceLastScan = 0;
        return;
    }

    if (state == "DISCONNECTED" && secondsSinceLastScan >= 5)
    {
        // scan for hotspots
        didScan = true;
        secondsSinceLastScan = 0;
        start_scanning(ctrl);
    }

    std::cout << "wifi state: " << state << std::endl;
    secondsSinceLastScan++;
}

int main()
{
    std::cout << "cartrackerd by jluims" << std::endl;
    wpa_ctrl *ctrl = wpa_ctrl_open("/var/run/wpa_supplicant/wlx984827e72b3d");
    if (ctrl == nullptr)
    {
        std::cout << "Failed to open wpa_supplicant control interface" << std::endl;
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