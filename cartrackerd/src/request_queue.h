#pragma once

#include "wifi_network.h"
#include <vector>
#include <map>

struct PendingRequest {
  time_t createdAt;
  std::vector<WifiNetwork> networks;
};

class RequestQueue {
private:
  long long idIndex;
  std::map<int, PendingRequest> requests;
public:
  int add_request(std::vector<WifiNetwork> networks);
  void remove_request(int id);
  void clear();
  bool empty();
  std::vector<PendingRequest> get_requests();
};