#include "api.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include "request_queue.h"
#include "curl/curl.h"

using json = nlohmann::json;

struct ServerConfig
{
  int interval;
};

struct ServerResponse
{
  ServerConfig serverConfig;
};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool API::sendNetworks(std::string baseUrl, RequestQueue networks, ConfigData &config)
{
  try
  {

    json j = json::array();

    for (PendingRequest request : networks.get_requests())
    {
      json obj;
      obj["createdAt"] = time(NULL);

      json aps = json::array();
      for (WifiNetwork network : request.networks)
      {
        json net;
        net["ssid"] = network.ssid.value_or("");
        net["bssid"] = network.bssid;
        net["flags"] = network.flags;
        net["frequency"] = network.frequency;
        net["rssi"] = network.rssi;
        aps.push_back(net);
      }

      obj["aps"] = aps;
      j.push_back(obj);
    }

    CURL *curl = curl_easy_init();
    if (!curl)
    {
      std::cout << "Failed to initialize curl" << std::endl;
      return false;
    }

    std::string fullUrl = std::string(baseUrl) + "/api/v1/data";
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4); // force ipv4

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: cartracker/1.0 by jluims");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string dump = j.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dump.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dump.length());

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, true);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      std::cout << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      return false;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (response_code != 200)
    {
      std::cout << "Bad response code: " << response_code << std::endl;
      return false;
    }

    std::string resStr(readBuffer);
    std::cout << "Raw response: " << resStr << std::endl;
    std::cout << "Response length: " << resStr.length() << std::endl;

    json body = json::parse(resStr);

    json serverConfig = body.at("serverConfig").get<json>();
    int interval = serverConfig.at("interval").get<int>();

    config.interval = interval;
  }
  catch (...)
  {
    std::cout << "failed to send networks!" << std::endl;
  }

  return true;
}