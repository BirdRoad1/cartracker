#pragma once

#include <vector>
#include "wifi_network.h"

class WifiHelper
{
public:
    bool scan(std::vector<WifiNetwork> &results);
    bool connect(const std::string& interface, const WifiNetwork &network);
    void disconnect(const std::string& interface);
    bool try_connect_hotspot(std::vector<WifiNetwork> networks, const std::vector<std::string> &hotspotNames, const std::string &interface);
};