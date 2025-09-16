#include "wifi_blocker.h"
#include <stdlib.h>

bool WifiBlocker::blocked = true; // start blocked just incase

void WifiBlocker::block() {
  system("rfkill block wifi");
  blocked = true;
}

void WifiBlocker::unblock() {
  system("rfkill unblock wifi");
  blocked = false;
}

bool WifiBlocker::isBlocked() {
  return blocked;
}