#include <Feen.h>

void setup() {
  Serial.begin(115200);

  if (!Feen.begin()) {
    Serial.println("Feen display init failed");
    return;
  }

  auto& lcd = Feen.display();
  lcd.fillScreen(feen::color::BLACK);
  lcd.drawText(24, 42, "FEEN", feen::color::GOLD, feen::color::BLACK, 4);
  lcd.drawText(24, 92, "FRAMEWORK", feen::color::WHITE, feen::color::BLACK, 2);
  lcd.drawText(24, 126, Feen.boardName(), feen::color::GREEN, feen::color::BLACK, 1);
}

void loop() {
  Feen.update();
  delay(2);
}
