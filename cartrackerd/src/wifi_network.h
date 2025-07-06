#pragma once

#include <optional>
#include <string>

struct WifiNetwork
{
    std::string bssid;
    int frequency;
    int rssi;
    std::string flags;
    std::optional<std::string> ssid;
};