#pragma once

#include <vector>
#include "wifi_network.h"

class WifiHelper
{
private:
    void init(const std::string &interface);
    void parse_scan_output(std::string output, std::vector<WifiNetwork> &outNetworks);

public:
    bool scan(const std::string &interface, std::vector<WifiNetwork> &results);
    bool connect(const std::string &interface, const WifiNetwork &network);
    void disconnect(const std::string &interface);
    bool try_connect_hotspot(std::vector<WifiNetwork> networks, const std::vector<std::string> &hotspotNames, const std::string &interface);
};