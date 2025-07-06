#pragma once

#include <vector>
#include "wifi_network.h"
#include "wpa_ctrl.h"

class WifiHelper
{
private:
    bool did_init = false;
    bool check_init();
    static void event_thread(wpa_ctrl* ctrl, void (*callback)(std::string));
    wpa_ctrl *ctrl;

public:
    bool init(std::string interface);
    bool init_events(std::string interface, void (*callback)(std::string));
    int send_request(std::string request, std::string &response, void (*msg_cb)(char *msg, size_t len));
    void connect(const WifiNetwork &network);
    void try_connect_hotspot(std::vector<WifiNetwork> networks);
    bool start_scanning();
    bool get_scan_results(std::vector<WifiNetwork> &results);
    bool get_wifi_state(std::string &out);
};