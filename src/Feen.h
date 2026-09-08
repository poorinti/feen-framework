#pragma once
#include <Arduino.h>
#include "BeeDisplay.h"
#include "BeeWiFi.h"
#include "BeeNeXTBoard.h"

namespace feen {

using Display = beenext::BeeDisplay;
using WiFiManager = beenext::BeeWiFi;

namespace color {
  static constexpr uint16_t BLACK  = beenext::color::BLACK;
  static constexpr uint16_t WHITE  = beenext::color::WHITE;
  static constexpr uint16_t GOLD   = beenext::color::GOLD;
  static constexpr uint16_t YELLOW = beenext::color::YELLOW;
  static constexpr uint16_t GREEN  = beenext::color::GREEN;
  static constexpr uint16_t RED    = beenext::color::RED;
  static constexpr uint16_t DARK   = beenext::color::DARK;
  static constexpr uint16_t GRID   = beenext::color::GRID;
  static constexpr uint16_t GRAY   = beenext::color::GRAY;
  static constexpr uint16_t PANEL  = beenext::color::PANEL;
}

class Framework {
public:
  bool begin();
  void update();
  void connectWiFi(const char* ssid, const char* password);

  Display& display() { return display_; }
  WiFiManager& wifi() { return wifi_; }
  const char* boardName() const { return "BeeNeXT 2.8R"; }
  bool ready() const { return ready_; }

private:
  Display display_;
  WiFiManager wifi_;
  bool ready_ = false;
  bool wifiStarted_ = false;
};

extern Framework Feen;

} // namespace feen

using feen::Feen;
