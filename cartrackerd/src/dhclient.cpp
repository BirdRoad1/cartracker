#include "dhclient.h"

void DhClient::release(const std::string &interface)
{
  std::string cmd = "dhclient -r " + interface;
  system(cmd.c_str());
}

void DhClient::renew(const std::string &interface)
{
  std::string cmd = "dhclient -4 -1 -v " + interface;
  system(cmd.c_str());
}