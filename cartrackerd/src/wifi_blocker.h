class WifiBlocker {
private:
  static bool blocked;
public:
  static void block();
  static void unblock();
  static bool isBlocked();
};