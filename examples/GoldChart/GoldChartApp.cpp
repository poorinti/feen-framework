#include "GoldChartApp.h"

namespace feen {

GoldChartApp::GoldChartApp(Display& display) : lcd_(display) {}

void GoldChartApp::drawGrid() {
  lcd_.fillRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, color::DARK);
  lcd_.drawRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, color::GRID);
  for (int i = 1; i < 4; ++i) {
    int y = GRAPH_Y + (GRAPH_H * i) / 4;
    for (int x = GRAPH_X + 1; x < GRAPH_X + GRAPH_W - 1; x += 5)
      lcd_.drawFastHLine(x, y, 2, color::GRID);
  }
  for (int i = 1; i < 6; ++i) {
    int x = GRAPH_X + (GRAPH_W * i) / 6;
    for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y += 5)
      lcd_.drawFastVLine(x, y, 2, color::GRID);
  }
}

void GoldChartApp::drawChrome() {
  lcd_.fillScreen(color::BLACK);
  lcd_.fillRect(0, 0, lcd_.width(), 34, color::PANEL);
  lcd_.drawText(12, 9, "FEEN GOLD", color::GOLD, color::PANEL, 2);
  lcd_.drawText(16, 48, "THB", color::GRAY, color::BLACK, 1);
  lcd_.drawText(18, 91, "SIMULATED PRICE / DISPLAY TEST", color::GRAY, color::BLACK, 1);
  drawGrid();
}

void GoldChartApp::drawPrice() {
  lcd_.fillRect(12, 58, 190, 30, color::BLACK);
  char buf[16];
  snprintf(buf, sizeof(buf), "%05d", price_);
  lcd_.drawText(16, 61, buf, color::WHITE, color::BLACK, 3);
}

void GoldChartApp::drawGraph() {
  drawGrid();
  int32_t minV = samples_[0], maxV = samples_[0];
  for (int i = 1; i < NPTS; ++i) {
    if (samples_[i] < minV) minV = samples_[i];
    if (samples_[i] > maxV) maxV = samples_[i];
  }
  if (maxV - minV < 20) { maxV += 10; minV -= 10; }
  minV -= 8; maxV += 8;

  for (int i = 1; i < NPTS; ++i) {
    const int x0 = GRAPH_X + 2 + (i - 1) * (GRAPH_W - 5) / (NPTS - 1);
    const int x1 = GRAPH_X + 2 + i * (GRAPH_W - 5) / (NPTS - 1);
    const int y0 = GRAPH_Y + GRAPH_H - 3 -
      int32_t(samples_[i - 1] - minV) * (GRAPH_H - 6) / (maxV - minV);
    const int y1 = GRAPH_Y + GRAPH_H - 3 -
      int32_t(samples_[i] - minV) * (GRAPH_H - 6) / (maxV - minV);
    lcd_.drawLine(x0, y0, x1, y1, color::GOLD);
    lcd_.drawLine(x0, y0 + 1, x1, y1 + 1, color::YELLOW);
  }
}

void GoldChartApp::begin() {
  randomSeed((uint32_t)esp_random());
  int32_t v = 51200;
  for (int i = 0; i < NPTS; ++i) {
    v += random(-16, 17);
    samples_[i] = v;
  }
  price_ = samples_[NPTS - 1];
  drawChrome();
  drawPrice();
  drawGraph();
}

void GoldChartApp::update(uint32_t nowMs) {
  if (nowMs - lastUpdate_ < 1500) return;
  lastUpdate_ = nowMs;

  for (int i = 0; i < NPTS - 1; ++i) samples_[i] = samples_[i + 1];
  samples_[NPTS - 1] = constrain(samples_[NPTS - 2] + random(-20, 21), 51000, 51450);
  price_ = samples_[NPTS - 1];
  drawPrice();
  drawGraph();
  Serial.printf("demo gold = %d THB\n", price_);
}

} // namespace feen
