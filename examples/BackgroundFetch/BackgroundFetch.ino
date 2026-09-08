#include <Feen.h>

static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Plain HTTP keeps this first example certificate-free. For HTTPS, configure
// a CA certificate with setCACert(), or use setInsecureHttps(true) only while
// testing on a trusted network.
static const char* API_URL = "http://example.com/";

void setup() {
  Serial.begin(115200);
  Feen.begin();
  Feen.connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  // Runs GET on a dedicated low-priority FreeRTOS task pinned to core 0.
  // The main loop remains free for display, touch and application logic.
  Feen.fetch().timeout(8000);
  Feen.fetch().maxBodyBytes(4096);
  Feen.fetch().begin(API_URL, 10000);
}

void loop() {
  Feen.update();

  feen::FetchResult result;
  if (Feen.fetch().read(result)) {
    Serial.printf("HTTP %d, bytes=%u, truncated=%s\n",
                  result.statusCode,
                  static_cast<unsigned>(result.body.length()),
                  result.truncated ? "yes" : "no");

    if (result.ok()) {
      Serial.println(result.body);
    } else {
      Serial.printf("Fetch error: %s\n", result.error.c_str());
    }
  }

  // UI/touch work can run here continuously while the request is in progress.
  delay(2);
}
