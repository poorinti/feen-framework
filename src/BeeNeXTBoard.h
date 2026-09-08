#pragma once
#include <Arduino.h>

namespace beenext {

struct Board28R {
  static constexpr int TFT_MISO = 12;
  static constexpr int TFT_MOSI = 13;
  static constexpr int TFT_SCLK = 14;
  static constexpr int TFT_CS   = 15;
  static constexpr int TFT_DC   = 2;
  static constexpr int TFT_RST  = -1;
  static constexpr int TFT_BL   = 21;

  static constexpr int TOUCH_INT  = 36;
  static constexpr int TOUCH_MOSI = 32;
  static constexpr int TOUCH_MISO = 39;
  static constexpr int TOUCH_SCLK = 25;
  static constexpr int TOUCH_CS   = 33;

  static constexpr int SD_SCLK = 18;
  static constexpr int SD_MISO = 19;
  static constexpr int SD_MOSI = 23;
  static constexpr int SD_CS   = 5;

  static constexpr int LDR_PIN     = 34;
  static constexpr int SPEAKER_PIN = 26;
  static constexpr int LED_R       = 4;
  static constexpr int LED_G       = 16;
  static constexpr int LED_B       = 17;

  static constexpr int LCD_W = 320;
  static constexpr int LCD_H = 240;
};

namespace color {
  static constexpr uint16_t BLACK = 0x0000;
  static constexpr uint16_t WHITE = 0xFFFF;
  static constexpr uint16_t GOLD  = 0xFEA0;
  static constexpr uint16_t YELLOW = 0xFFE0;
  static constexpr uint16_t GREEN = 0x07E0;
  static constexpr uint16_t RED   = 0xF800;
  static constexpr uint16_t DARK  = 0x1082;
  static constexpr uint16_t GRID  = 0x31A6;
  static constexpr uint16_t GRAY  = 0x8410;
  static constexpr uint16_t PANEL = 0x18E3;
}

} // namespace beenext
