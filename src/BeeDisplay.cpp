#include "BeeDisplay.h"

namespace beenext {

BeeDisplay::BeeDisplay() : spi_(HSPI) {}

static inline void csLow()  { digitalWrite(Board28R::TFT_CS, LOW); }
static inline void csHigh() { digitalWrite(Board28R::TFT_CS, HIGH); }
static inline void dcCmd()  { digitalWrite(Board28R::TFT_DC, LOW); }
static inline void dcData() { digitalWrite(Board28R::TFT_DC, HIGH); }

void BeeDisplay::writeCommand(uint8_t cmd) {
  spi_.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  csLow(); dcCmd(); spi_.transfer(cmd); csHigh();
  spi_.endTransaction();
}

void BeeDisplay::writeCommandData(uint8_t cmd, const uint8_t* data, size_t len) {
  spi_.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  csLow();
  dcCmd(); spi_.transfer(cmd);
  dcData();
  for (size_t i = 0; i < len; ++i) spi_.transfer(data[i]);
  csHigh();
  spi_.endTransaction();
}

void BeeDisplay::setAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  uint8_t d[4];
  d[0] = uint8_t(x0 >> 8); d[1] = uint8_t(x0);
  d[2] = uint8_t(x1 >> 8); d[3] = uint8_t(x1);
  writeCommandData(0x2A, d, 4);
  d[0] = uint8_t(y0 >> 8); d[1] = uint8_t(y0);
  d[2] = uint8_t(y1 >> 8); d[3] = uint8_t(y1);
  writeCommandData(0x2B, d, 4);
  writeCommand(0x2C);
}

void BeeDisplay::pushColorRepeat(uint16_t color, uint32_t count) {
  const uint8_t hi = color >> 8;
  const uint8_t lo = color & 0xFF;
  spi_.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  csLow(); dcData();
  while (count--) { spi_.transfer(hi); spi_.transfer(lo); }
  csHigh();
  spi_.endTransaction();
}

bool BeeDisplay::begin() {
  pinMode(Board28R::TFT_CS, OUTPUT);
  pinMode(Board28R::TFT_DC, OUTPUT);
  pinMode(Board28R::TFT_BL, OUTPUT);
  csHigh();
  backlight(false);

  spi_.begin(Board28R::TFT_SCLK, Board28R::TFT_MISO,
             Board28R::TFT_MOSI, Board28R::TFT_CS);

  writeCommand(0x01); delay(150);
  writeCommand(0x28);

  const uint8_t ef[] = {0x03,0x80,0x02}; writeCommandData(0xEF, ef, sizeof(ef));
  const uint8_t cf[] = {0x00,0xC1,0x30}; writeCommandData(0xCF, cf, sizeof(cf));
  const uint8_t ed[] = {0x64,0x03,0x12,0x81}; writeCommandData(0xED, ed, sizeof(ed));
  const uint8_t e8[] = {0x85,0x00,0x78}; writeCommandData(0xE8, e8, sizeof(e8));
  const uint8_t cb[] = {0x39,0x2C,0x00,0x34,0x02}; writeCommandData(0xCB, cb, sizeof(cb));
  const uint8_t f7[] = {0x20}; writeCommandData(0xF7, f7, 1);
  const uint8_t ea[] = {0x00,0x00}; writeCommandData(0xEA, ea, 2);
  const uint8_t p1[] = {0x23}; writeCommandData(0xC0, p1, 1);
  const uint8_t p2[] = {0x10}; writeCommandData(0xC1, p2, 1);
  const uint8_t vm1[] = {0x3E,0x28}; writeCommandData(0xC5, vm1, 2);
  const uint8_t vm2[] = {0x86}; writeCommandData(0xC7, vm2, 1);

  // BeeNeXT 2.8R non-mirrored landscape: MV + BGR.
  // 0xB8 mirrored the framebuffer horizontally on this physical panel.
  const uint8_t madctl[] = {0x28}; writeCommandData(0x36, madctl, 1);
  const uint8_t pixfmt[] = {0x55}; writeCommandData(0x3A, pixfmt, 1);
  const uint8_t fr[] = {0x00,0x18}; writeCommandData(0xB1, fr, 2);
  const uint8_t dfc[] = {0x08,0x82,0x27}; writeCommandData(0xB6, dfc, 3);
  const uint8_t gamma[] = {0x01}; writeCommandData(0xF2, gamma, 1);
  writeCommandData(0x26, gamma, 1);

  writeCommand(0x11); delay(120);
  writeCommand(0x29); delay(20);
  backlight(true);
  return true;
}

void BeeDisplay::backlight(bool on) {
  digitalWrite(Board28R::TFT_BL, on ? HIGH : LOW);
}

void BeeDisplay::fillScreen(uint16_t color) {
  fillRect(0, 0, Board28R::LCD_W, Board28R::LCD_H, color);
}

void BeeDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {
  if (w <= 0 || h <= 0 || x >= width() || y >= height()) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > width()) w = width() - x;
  if (y + h > height()) h = height() - y;
  if (w <= 0 || h <= 0) return;
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  pushColorRepeat(c, uint32_t(w) * uint32_t(h));
}

void BeeDisplay::drawPixel(int16_t x, int16_t y, uint16_t c) {
  if (x < 0 || x >= width() || y < 0 || y >= height()) return;
  fillRect(x, y, 1, 1, c);
}

void BeeDisplay::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) { fillRect(x, y, w, 1, c); }
void BeeDisplay::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) { fillRect(x, y, 1, h, c); }

void BeeDisplay::drawLine(int x0, int y0, int x1, int y1, uint16_t c) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  for (;;) {
    drawPixel(x0, y0, c);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void BeeDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {
  drawFastHLine(x, y, w, c);
  drawFastHLine(x, y + h - 1, w, c);
  drawFastVLine(x, y, h, c);
  drawFastVLine(x + w - 1, y, h, c);
}

const uint8_t* BeeDisplay::glyph(char c) const {
  static const uint8_t blank[5] PROGMEM={0,0,0,0,0};
  static const uint8_t digits[10][5] PROGMEM={
    {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E}
  };
  static const uint8_t A[5] PROGMEM={0x7E,0x11,0x11,0x11,0x7E};
  static const uint8_t B[5] PROGMEM={0x7F,0x49,0x49,0x49,0x36};
  static const uint8_t C[5] PROGMEM={0x3E,0x41,0x41,0x41,0x22};
  static const uint8_t D[5] PROGMEM={0x7F,0x41,0x41,0x22,0x1C};
  static const uint8_t E[5] PROGMEM={0x7F,0x49,0x49,0x49,0x41};
  static const uint8_t F[5] PROGMEM={0x7F,0x09,0x09,0x09,0x01};
  static const uint8_t G[5] PROGMEM={0x3E,0x41,0x49,0x49,0x7A};
  static const uint8_t H[5] PROGMEM={0x7F,0x08,0x08,0x08,0x7F};
  static const uint8_t I[5] PROGMEM={0x00,0x41,0x7F,0x41,0x00};
  static const uint8_t K[5] PROGMEM={0x7F,0x08,0x14,0x22,0x41};
  static const uint8_t L[5] PROGMEM={0x7F,0x40,0x40,0x40,0x40};
  static const uint8_t M[5] PROGMEM={0x7F,0x02,0x0C,0x02,0x7F};
  static const uint8_t O[5] PROGMEM={0x3E,0x41,0x41,0x41,0x3E};
  static const uint8_t P[5] PROGMEM={0x7F,0x09,0x09,0x09,0x06};
  static const uint8_t R[5] PROGMEM={0x7F,0x09,0x19,0x29,0x46};
  static const uint8_t S[5] PROGMEM={0x46,0x49,0x49,0x49,0x31};
  static const uint8_t T[5] PROGMEM={0x01,0x01,0x7F,0x01,0x01};
  static const uint8_t U[5] PROGMEM={0x3F,0x40,0x40,0x40,0x3F};
  static const uint8_t V[5] PROGMEM={0x1F,0x20,0x40,0x20,0x1F};
  static const uint8_t W[5] PROGMEM={0x3F,0x40,0x38,0x40,0x3F};
  static const uint8_t Y[5] PROGMEM={0x07,0x08,0x70,0x08,0x07};
  static const uint8_t plus[5] PROGMEM={0x08,0x08,0x3E,0x08,0x08};
  static const uint8_t minus_[5] PROGMEM={0x08,0x08,0x08,0x08,0x08};
  static const uint8_t dot[5] PROGMEM={0x00,0x60,0x60,0x00,0x00};
  static const uint8_t slash[5] PROGMEM={0x20,0x10,0x08,0x04,0x02};

  if (c >= '0' && c <= '9') return digits[c-'0'];
  switch(c) {
    case 'A': return A; case 'B': return B; case 'C': return C; case 'D': return D; case 'E': return E; case 'F': return F;
    case 'G': return G; case 'H': return H; case 'I': return I; case 'K': return K; case 'L': return L;
    case 'M': return M; case 'O': return O; case 'P': return P; case 'R': return R;
    case 'S': return S; case 'T': return T; case 'U': return U; case 'V': return V;
    case 'Y': return Y; case '+': return plus; case '-': return minus_;
    case '.': return dot; case '/': return slash; default: return blank;
  }
}

void BeeDisplay::drawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
  const uint8_t* g = glyph(c);
  for (int col = 0; col < 5; ++col) {
    uint8_t bits = pgm_read_byte(g + col);
    for (int row = 0; row < 7; ++row) {
      fillRect(x + col * scale, y + row * scale, scale, scale,
               (bits & (1 << row)) ? fg : bg);
    }
  }
  fillRect(x + 5 * scale, y, scale, 7 * scale, bg);
}

void BeeDisplay::drawText(int16_t x, int16_t y, const char* s, uint16_t fg, uint16_t bg, uint8_t scale) {
  while (*s) {
    drawChar(x, y, *s++, fg, bg, scale);
    x += 6 * scale;
  }
}

} // namespace beenext
