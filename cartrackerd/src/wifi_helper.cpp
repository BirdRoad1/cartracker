#include "wifi_helper.h"
#include <iostream>
#include <regex>
#include <filesystem>
#include <optional>
#include <exception>
#include <thread>
#include <vector>

#define BUFFER_LEN 10000

std::string run_cmd(const std::string &cmd)
{
  char buffer[128] = {};
  std::string result;

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
  {
    throw std::runtime_error("popen failed");
  }

  while (fgets(buffer, 128, pipe) != nullptr)
  {
    result += buffer;
  }

  int status = pclose(pipe);
  if (status == -1)
  {
    throw std::runtime_error("pclose failed");
  }

  return result;
}

void WifiHelper::init(const std::string& interface) {
  run_cmd("ip link set " + interface + " up");
}

void WifiHelper::parse_scan_output(std::string output, std::vector<WifiNetwork> &outNetworks)
{
  std::stringstream ss(output);
  std::string line;

  WifiNetwork network;
  std::string currentWifiBssid = "";
  std::optional<std::string> currentWifiSsid = "";
  int currentWifiRssi = 1000;

  std::vector<WifiNetwork> networks;

  while (std::getline(ss, line, '\n'))
  {
    if (line.starts_with("BSS"))
    {
      if (!currentWifiBssid.empty())
      {
        WifiNetwork newNetwork{
            .bssid = currentWifiBssid,
            .frequency = 0,
            .rssi = currentWifiRssi,
            .flags = "",
            .ssid = currentWifiSsid};

        networks.push_back(newNetwork);
      }
      currentWifiBssid = "";
      currentWifiSsid.reset();
      currentWifiRssi = 1000;

      currentWifiBssid = line.substr(4, 17);
    }

    if (currentWifiBssid.empty())
      continue;

    if (line.starts_with("\tSSID: "))
    {
      currentWifiSsid = line.substr(7, line.size() - 7);
    }

    if (line.starts_with("\tsignal: "))
    {
      std::regex exp("-(\\d*)\\.\\d* dBm");
      std::smatch match;
      if (std::regex_search(line, match, exp))
      {
        // TODO: safe?
        currentWifiRssi = std::stoi(match[0]);
      }
    }
  }

  if (!currentWifiBssid.empty())
  {
    WifiNetwork newNetwork{
        .bssid = currentWifiBssid,
        .frequency = 0,
        .rssi = currentWifiRssi,
        .flags = "",
        .ssid = currentWifiSsid};

    networks.push_back(newNetwork);
  }

  outNetworks = networks;
}

bool WifiHelper::scan(const std::string& interface, std::vector<WifiNetwork> &results)
{
  init(interface);
  std::string out = run_cmd("/usr/sbin/iw dev " + interface + " scan");
  parse_scan_output(out, results);

  return true;
}

bool WifiHelper::connect(const std::string& interface, const WifiNetwork &network)
{
  if (!network.ssid.has_value()) return false;
  init(interface);

  std::string out = run_cmd("iw dev " + interface + " connect -w \"" + network.ssid.value() + "\"");
  return out.contains("connected to");
}

void WifiHelper::disconnect(const std::string& interface)
{
  init(interface);
  run_cmd("iw dev " + interface + " disconnect");
}

bool WifiHelper::try_connect_hotspot(std::vector<WifiNetwork> networks, const std::vector<std::string> &hotspotNames, const std::string &interface)
{
  std::cout << "Try connect?" << std::endl;
  WifiNetwork *xfinityNetwork = nullptr;
  int rssid = -10000;
  for (WifiNetwork &network : networks)
  {
    if (network.ssid.has_value())
    {
      std::cout << network.ssid.value() << std::endl;
    }

    if (network.ssid.has_value() && std::find(hotspotNames.begin(), hotspotNames.end(), network.ssid.value()) != hotspotNames.end() && network.rssi > rssid)
    {
      std::cout << "Found matching network ssid: " << network.ssid.value() << std::endl;
      // Try connect
      rssid = network.rssi;
      xfinityNetwork = &network;
    }
  }

  if (xfinityNetwork == 0)
  {
    std::cout << "No xfinity network found" << std::endl;
    return false;
  }

  printf("Address of xfinityNetwork: %p\n", (void *)xfinityNetwork);
  std::cout << "DA SSID: " << xfinityNetwork->ssid.value() << std::endl;

  connect(interface, *xfinityNetwork);
  printf("Connected probably! Requesting ip!\n");
  return true;
}
