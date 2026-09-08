#pragma once
#include <Arduino.h>
#include <Feen.h>

namespace feen {

class GoldChartApp {
public:
  explicit GoldChartApp(Display& display);
  void begin();
  void update(uint32_t nowMs);
  int currentPrice() const { return price_; }

private:
  static constexpr int GRAPH_X = 18;
  static constexpr int GRAPH_Y = 104;
  static constexpr int GRAPH_W = 284;
  static constexpr int GRAPH_H = 112;
  static constexpr int NPTS = 60;

  Display& lcd_;
  int32_t samples_[NPTS]{};
  int price_ = 51200;
  uint32_t lastUpdate_ = 0;

  void drawChrome();
  void drawGrid();
  void drawGraph();
  void drawPrice();
};

} // namespace feen
