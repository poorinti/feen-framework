#include <Feen.h>

static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);
  Feen.begin();
  Feen.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  Feen.update();

  static uint32_t last = 0;
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("WiFi=%s IP=%s RSSI=%ld\n",
                  Feen.wifi().stateText(),
                  Feen.wifi().ip().toString().c_str(),
                  (long)Feen.wifi().rssi());
  }
}
