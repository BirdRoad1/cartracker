#pragma once
#include <string>
#include <filesystem>
#include <vector>

struct ConfigData
{
    double interval;
    std::string serverUrl;
    std::string interface;
    std::vector<std::string> hotspotNames;
};

class Config
{
public:
    static bool read(std::filesystem::path configFile, ConfigData &config);
};
