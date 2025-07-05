#include "wifi_helper.h"
#include <iostream>
#include <regex>
#include <filesystem>
#include <optional>
#include <exception>

#define BUFFER_LEN 4096

bool WifiHelper::init(std::string interface) {
    std::string path("/var/run/wpa_supplicant/" + interface);
    wpa_ctrl *ctrl = wpa_ctrl_open(path.c_str());
    if (ctrl == nullptr)
    {
        std::cout << "failed to open wpa_supplicant control interface." << std::endl;
        std::cout << "please ensure the interface '" << interface << "' exists and wpa_supplicant is running!" << std::endl;
        return false;
    }

    this->ctrl = ctrl;
    did_init = true;

    return true;
}

bool WifiHelper::check_init() {
    if (!did_init) {
        throw std::runtime_error("did not init!");
        return false;
    }

    return true;
}

bool WifiHelper::start_scanning()
{
    if (!check_init()) return false;
    std::string res;

    if (send_request("SCAN", res, [](char *msg, size_t len)
                     { std::cout << "we got a callback!" << std::endl; }) == -1)
    {
        return false;
    }

    return true;
}

void WifiHelper::connect(const WifiNetwork& network) {
    if (!check_init()) return;
    if (!network.ssid.has_value()) {
        std::cout << "cannot connect to network without ssid" << std::endl;
        return;
    }

    std::string ssid = network.ssid.value();
    std::string bssid = network.bssid;
    std::cout << "BSSID: " << bssid << std::endl;

    std::string setRes;
    // Remove all networks to prevent bloat
    if (send_request("REMOVE_NETWORK all", setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (remove all networks)" << std::endl;
        return;
    }

    std::string networkId;
    if (send_request("ADD_NETWORK", networkId, nullptr) == -1) {
        std::cout << "failed to connect (add network)" << std::endl;
        std::cout << networkId << std::endl;
        return;
    }

    if (send_request(std::string("SET_NETWORK ") + networkId + " ssid \"" + ssid + "\"", setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (set network ssid)" << std::endl;
        return;
    }

    if (send_request(std::string("SET_NETWORK ") + networkId + " bssid " + bssid, setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (set network bssid)" << std::endl;
        return;
    }

    if (send_request(std::string("SET_NETWORK ") + networkId + " key_mgmt NONE", setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (set key_mgmt)" << std::endl;
        return;
    }

    if (send_request(std::string("ENABLE_NETWORK ") + networkId, setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (enable network)" << std::endl;
        return;
    }

    if (send_request(std::string("SELECT_NETWORK ") + networkId, setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (select network)" << std::endl;
        return;
    }
    
    if (send_request("REASSOCIATE", setRes, nullptr) == -1 || setRes != "OK\n") {
        std::cout << "failed to connect (reassociate)" << std::endl;
        return;
    }

    std::cout << "setRes: " << setRes << std::endl;
}

void WifiHelper::try_connect_hotspot(std::vector<WifiNetwork> networks)
{
    if (!check_init()) return;
    std::cout << "Try connect?" << std::endl;
    WifiNetwork* xfinityNetwork;
    int rssid = -10000;
    for (WifiNetwork& network : networks)
    {
        if (!network.ssid.has_value())
            continue;

        if (network.ssid.value() == "xfinitywifi" && network.rssid > rssid)
        {
            // Try connect
            rssid = network.rssid;
            xfinityNetwork = &network;
        }
    }

    if (xfinityNetwork == 0) {
        std::cout << "No xfinity network found" << std::endl;
        return;
    } 
    
    connect(*xfinityNetwork);
}


int WifiHelper::send_request(std::string request, std::string &response, void (*msg_cb)(char *msg, size_t len))
{
    if (!check_init()) return -1;
    std::cout << "[DEBUG] Sending request: " << request << std::endl;

    std::string buffer(BUFFER_LEN, '\0');
    size_t reply_len = buffer.size();

    int result = wpa_ctrl_request(ctrl, request.c_str(), request.size(), &buffer[0], &reply_len, msg_cb);

    if (result != 0)
    {
        return result;
    }

    buffer.resize(reply_len);

    response = buffer;
    return result;
}

bool WifiHelper::get_wifi_state(std::string &out)
{
    if (!check_init()) return false;

    std::string res;
    if (send_request("STATUS", res, nullptr) != 0)
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

bool WifiHelper::get_scan_results(std::vector<WifiNetwork> &results)
{
    if (!check_init()) return false;
    
    std::string res;

    if (send_request("SCAN_RESULTS", res, nullptr) == -1)
    {
        return false;
    }

    std::regex scanRegex("^([a-fA-F0-9:]{17})\\t(\\d+)\\t(-?\\d+)\\t([^\\t]*)\\t?([^\\t\\n]*)$");

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