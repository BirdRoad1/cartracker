#include <string>

class DhClient {
public:
  static void release(const std::string& interface);
  static void renew(const std::string& interface);
};