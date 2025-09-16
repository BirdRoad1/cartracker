#pragma once

class WpaService {
private:
  static int pid;
public:
  static bool start(const char* interface);
  static bool stop();
  static bool isRunning();

};