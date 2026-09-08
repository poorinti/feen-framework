#include <Feen.h>
#include "GoldChartApp.h"

using namespace feen;

GoldChartApp app(Feen.display());

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("Feen Framework / GoldChart demo");

  if (!Feen.begin()) {
    Serial.println("Display init failed");
    return;
  }

  app.begin();
}

void loop() {
  Feen.update();
  app.update(millis());
  delay(2);
}
