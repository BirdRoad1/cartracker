#pragma once

#include <vector>
#include "wifi_network.h"
#include <string>

class API {
public:
    static bool sendNetworks(std::string baseUrl, std::vector<WifiNetwork> networks);
};