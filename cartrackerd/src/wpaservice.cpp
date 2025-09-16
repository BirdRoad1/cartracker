#include "wpaservice.h"
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include "wifi_blocker.h"

int WpaService::pid = -1;

bool WpaService::start(const char *interface)
{
  std::cout << "Starting wpa_supplicant..." << std::endl;
  std::ofstream outputFile("/tmp/wpa_supplicant.conf");
  if (!outputFile.is_open())
  {
    std::cout << "Failed to create wpa_supplicant config file" << std::endl;
    return false;
  }

  outputFile << "ctrl_interface=DIR=/run/wpa_supplicant\ncountry=US\n";
  outputFile.flush();
  outputFile.close();

  pid_t PID = fork();
  if (PID == 0)
  {
    int fd = open("/dev/null", O_WRONLY);

    char *const args[] = {
        const_cast<char *>("/usr/sbin/wpa_supplicant"),
        const_cast<char *>("-i"),
        const_cast<char *>(interface),
        const_cast<char *>("-c"),
        const_cast<char *>("/tmp/wpa_supplicant.conf"),
        nullptr};

    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
    
    execvp("/usr/sbin/wpa_supplicant", args);

    perror("execvp failed");
    exit(1);
  }
  else if (PID > 0)
  {
    pid = PID;
    sleep(5); //TODO: detect wpa process ready
    return true;
  }
  else
  {
    perror("fork failed");
    return false;
  }
}

bool WpaService::stop()
{
  if (pid <= 0)
    return false;

  if (kill(pid, SIGTERM) != 0)
  {
    std::cout << "Failed to kill wpa_supplicant" << std::endl;
    return false;
  }

  waitpid(pid, nullptr, 0);
  pid = -1;

  return true;
}

bool WpaService::isRunning()
{
  return pid != -1;
}