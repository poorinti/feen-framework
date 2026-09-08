# Feen Framework

**Feen Framework** is a lightweight ESP32 framework for building applications on small display boards without depending on a large UI runtime. The first verified board profile is **BeeNeXT 2.8R**.

Repository: https://github.com/poorinti/feen-framework

The public API is intentionally small:

```cpp
#include <Feen.h>

void setup() {
  Feen.begin();
  Feen.display().fillScreen(feen::color::BLACK);
  Feen.connectWiFi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
}

void loop() {
  Feen.update();
}
```

## Current status

**Feen Framework 0.1.0** is working and has been tested on a physical BeeNeXT 2.8R board.

Current verified capabilities:

- `#include <Feen.h>` public API
- Standalone ILI9341 320x240 display driver
- Correct non-mirrored landscape orientation on BeeNeXT 2.8R
- Drawing primitives and built-in compact text renderer
- ESP32 Wi-Fi connect/reconnect service
- `HelloDisplay`, `WiFiStatus` and `GoldChart` examples
- All three examples compile with Arduino ESP32 core 2.0.13
- Physical flash/boot verification completed through CH340 / COM4
- Gold chart demo verified with 32-bit price storage
- Local Wi-Fi credentials and backup binaries excluded from Git

For the detailed development checkpoint and next-step list, see [`FEEN_STATUS.md`](FEEN_STATUS.md).

## Why Feen

- Simple `#include <Feen.h>` API
- Standalone ILI9341 display driver and drawing primitives
- ESP32 Wi-Fi connect/reconnect service
- Small core with application code kept in examples
- No BlynkGO runtime dependency
- Designed so more board profiles, touch, SD, OTA and widgets can be added later

## Verified hardware: BeeNeXT 2.8R

- MCU: ESP32-D0WD-V3, dual-core 240 MHz
- Flash: 4 MB
- LCD: ILI9341, 320x240 landscape
- LCD HSPI: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- XPT2046 touch reference pins: INT 36, MOSI 32, MISO 39, SCLK 25, CS 33
- SD reference pins: SCLK 18, MISO 19, MOSI 23, CS 5
- LDR 34, speaker 26, RGB LED 4/16/17

The display orientation has been validated on the physical BeeNeXT 2.8R panel and uses the non-mirrored landscape setting in `BeeDisplay.cpp`.

## Library layout

```text
Feen Framework/
├─ library.properties
├─ FEEN_STATUS.md
├─ src/
│  ├─ Feen.h / Feen.cpp        # public facade
│  ├─ BeeDisplay.*             # ILI9341 driver + graphics
│  ├─ BeeWiFi.*                # Wi-Fi service
│  └─ BeeNeXTBoard.h           # first board profile
└─ examples/
   ├─ HelloDisplay/
   ├─ WiFiStatus/
   └─ GoldChart/
```

## Examples

### HelloDisplay
Minimal display bring-up using only `Feen.begin()` and `Feen.display()`.

### WiFiStatus
Shows `Feen.connectWiFi()` and automatic reconnect. Replace the placeholder SSID/password locally before flashing. Do not commit private credentials.

### GoldChart
Animated 320x240 gold-price chart demo. It currently uses simulated price data so display and rendering can be tested independently of an external API.

## Arduino ESP32 build target

Verified with Arduino ESP32 core **2.0.13** on `ESP32 Dev Module`, 4 MB flash, DIO mode. The physical test board uses a CH340 serial bridge.

Typical current example size is about **54% flash** and **12–13% RAM**.

## Security

Wi-Fi secrets and local files are ignored by `.gitignore`. Do not commit full-flash backups or credential files.

## Roadmap

1. Core display + Feen facade — done
2. Wi-Fi service + reconnect — done
3. XPT2046 touch + calibration
4. Small reusable UI widgets (`Button`, `Label`, `Slider`, `Chart`)
5. Simple screen/navigation system
6. HTTPS/JSON data client
7. Live gold-price data source
8. SD storage and settings
9. OTA and recovery UI
10. Additional Feen board profiles
