#include "Feen.h"

namespace feen {

Framework Feen;

bool Framework::begin() {
  if (ready_) return true;
  ready_ = display_.begin();
  return ready_;
}

void Framework::connectWiFi(const char* ssid, const char* password) {
  wifi_.begin(ssid, password);
  wifiStarted_ = true;
}

void Framework::update() {
  if (wifiStarted_) wifi_.update();
}

} // namespace feen
