#pragma once

#include <vector>
#include "wifi_network.h"
#include "wpa_ctrl.h"
#include <mutex>

class WifiHelper
{
private:
    // bool did_init = false;
    bool check_init();
    static void event_thread(wpa_ctrl* ctrl, void (*callback)(std::string));
    // std::mutex ctrlMut;
    wpa_ctrl* ctrl = nullptr;

public:
    bool init(std::string interface);
    bool init_events(std::string interface, void (*callback)(std::string));
    int send_request(std::string request, std::string &response, void (*msg_cb)(char *msg, size_t len));
    bool connect(const WifiNetwork &network);
    void disconnect();
    bool try_connect_hotspot(std::vector<WifiNetwork> networks, const std::vector<std::string>& hotspotNames, const std::string& interface);
    bool start_scanning();
    bool get_scan_results(std::vector<WifiNetwork> &results);
    bool get_wifi_state(std::string &out);
    void close(const std::string& interface);
    bool is_ready();
};