#include <optional>
#include <string>

struct WifiNetwork
{
    std::string bssid;
    int frequency;
    int rssid;
    std::string flags;
    std::optional<std::string> ssid;
};