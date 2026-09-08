#pragma once
#include <Arduino.h>
#include <WiFi.h>

namespace beenext {

class BeeWiFi {
public:
  void begin(const char* ssid, const char* password);
  void update();
  bool connected() const;
  IPAddress ip() const;
  int32_t rssi() const;
  const char* stateText() const;

private:
  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  uint32_t lastAttempt_ = 0;
  bool announcedConnected_ = false;
  void reconnect();
};

} // namespace beenext
