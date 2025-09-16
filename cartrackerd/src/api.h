#pragma once

#include <vector>
#include "wifi_network.h"
#include "request_queue.h"
#include <string>
#include "config.h"

class API {
public:
    static bool sendNetworks(std::string baseUrl, RequestQueue networks, ConfigData& config);
};