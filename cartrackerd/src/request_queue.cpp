#include "request_queue.h"
#include <time.h>
#include <algorithm>

#define MAX_REQUESTS 100

int RequestQueue::add_request(std::vector<WifiNetwork> networks)
{
  time_t timestamp = time(NULL);

  if (requests.size() + 1 > MAX_REQUESTS) {
    requests.erase(requests.begin());
  }

  PendingRequest request;
  request.createdAt = timestamp;
  request.networks = networks;

  requests[++idIndex] = request;
  return idIndex;
}

bool RequestQueue::empty() {
  return this->requests.empty();
}

void RequestQueue::remove_request(int id)
{
  requests.erase(id);
}

void RequestQueue::clear() {
  requests.clear();
}

std::vector<PendingRequest> RequestQueue::get_requests()
{
  std::vector<PendingRequest> vec;
  for (const auto& pair : requests) {
    vec.push_back(pair.second);
  }

  return vec;
}