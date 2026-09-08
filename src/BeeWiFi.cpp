#include "BeeWiFi.h"

namespace beenext {

void BeeWiFi::begin(const char* ssid, const char* password) {
  ssid_ = ssid;
  password_ = password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  reconnect();
}

void BeeWiFi::reconnect() {
  if (!ssid_ || !password_) return;
  lastAttempt_ = millis();
  announcedConnected_ = false;
  Serial.printf("WiFi: connecting to %s\n", ssid_);
  WiFi.disconnect(false, false);
  WiFi.begin(ssid_, password_);
}

void BeeWiFi::update() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!announcedConnected_) {
      announcedConnected_ = true;
      Serial.printf("WiFi: connected, IP=%s, RSSI=%ld dBm\n",
                    WiFi.localIP().toString().c_str(), (long)WiFi.RSSI());
    }
    return;
  }

  if (millis() - lastAttempt_ >= 15000) reconnect();
}

bool BeeWiFi::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress BeeWiFi::ip() const {
  return WiFi.localIP();
}

int32_t BeeWiFi::rssi() const {
  return connected() ? WiFi.RSSI() : -127;
}

const char* BeeWiFi::stateText() const {
  return connected() ? "WIFI OK" : "WIFI...";
}

} // namespace beenext
