#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "BeeNeXTBoard.h"

namespace beenext {

class BeeDisplay {
public:
  BeeDisplay();

  bool begin();
  void backlight(bool on);
  void fillScreen(uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawText(int16_t x, int16_t y, const char* text, uint16_t color,
                uint16_t bg, uint8_t scale = 1);

  int16_t width() const { return Board28R::LCD_W; }
  int16_t height() const { return Board28R::LCD_H; }

private:
  SPIClass spi_;

  void writeCommand(uint8_t cmd);
  void writeCommandData(uint8_t cmd, const uint8_t* data, size_t len);
  void setAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
  void pushColorRepeat(uint16_t color, uint32_t count);
  void drawChar(int16_t x, int16_t y, char c, uint16_t color,
                uint16_t bg, uint8_t scale);
  const uint8_t* glyph(char c) const;
};

} // namespace beenext
