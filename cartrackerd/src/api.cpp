#include "api.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

bool API::sendNetworks(std::string baseUrl, std::vector<WifiNetwork> networks)
{
    json j = json::array();
    for (WifiNetwork network : networks)
    {
        json net;
        net["ssid"] = network.ssid.value_or("");
        net["bssid"] = network.bssid;
        net["flags"] = network.flags;
        net["frequency"] = network.frequency;
        net["rssi"] = network.rssi;
        j.push_back(net);
    }

    CURL *curl = curl_easy_init();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: cartracker/1.0 by jluims");

    std::string dump = j.dump();
    curl_easy_setopt(curl, CURLOPT_URL, baseUrl + "/api/data");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dump.c_str());

    CURLcode code;
    if ((code = curl_easy_perform(curl)) != CURLE_OK)
    {
        std::cout << "Failed to send request: " << curl_easy_strerror(code) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
}