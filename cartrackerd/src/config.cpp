#include "config.h"
#include <filesystem>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>

using json = nlohmann::json;

bool Config::read(std::filesystem::path configFile, ConfigData &config)
{
    std::ifstream reader(configFile);

    if (!reader.good())
    {
        return false;
    }

    try
    {
        json j;
        reader >> j;

        double interval = j.value("interval", 5);
        std::string serverUrl = j["serverUrl"].get<std::string>();
        std::string interface = j.value("interface", "wlan0");
        std::vector<std::string> hotspotNames = j["hotspot_names"];

        config = ConfigData{
            interval, serverUrl, interface, hotspotNames};
    }
    catch (json::exception &ex)
    {
        std::cout << "Failed to read " << std::filesystem::absolute(configFile) << ": " << ex.what() << std::endl;
        return false;
    }

    return true;
}