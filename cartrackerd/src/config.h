#pragma once
#include <string>
#include <filesystem>

struct ConfigData
{
    double interval;
    std::string serverUrl;
    std::string interface;
};

class Config
{
public:
    static bool read(std::filesystem::path configFile, ConfigData &config);
};
