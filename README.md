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

**Feen Framework 0.2.0** adds a non-blocking background HTTP/HTTPS fetch service while keeping the display/UI loop responsive.

Current verified capabilities:

- `#include <Feen.h>` public API
- Standalone ILI9341 320x240 display driver
- Correct non-mirrored landscape orientation on BeeNeXT 2.8R
- Drawing primitives and built-in compact text renderer
- ESP32 Wi-Fi connect/reconnect service
- Background HTTP/HTTPS GET on a dedicated low-priority FreeRTOS task pinned to core 0
- Thread-safe latest-result mailbox; the background task never draws directly to the UI
- Configurable interval, timeout, body-size cap, headers, CA certificate and explicit insecure HTTPS test mode
- `HelloDisplay`, `WiFiStatus`, `GoldChart` and `BackgroundFetch` examples
- BackgroundFetch and existing examples compile with Arduino ESP32 core 2.0.13
- Physical display/flash/boot verification completed through CH340 / COM4
- Physical BackgroundFetch smoke tests verified for both HTTP and HTTPS: repeated HTTP 200 responses while the main/UI loop continued emitting `LOOP_ALIVE` once per second; HTTPS body limiting also returned `truncated=true` at the configured cap
- Gold chart demo verified with 32-bit price storage
- Local Wi-Fi credentials and backup binaries excluded from Git

For the detailed development checkpoint and next-step list, see [`FEEN_STATUS.md`](FEEN_STATUS.md).

## Why Feen

- Simple `#include <Feen.h>` API
- Standalone ILI9341 display driver and drawing primitives
- ESP32 Wi-Fi connect/reconnect service
- Background HTTP/HTTPS fetch service that does not block the UI loop
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
│  ├─ FeenFetch.*              # background HTTP/HTTPS service
│  ├─ BeeDisplay.*             # ILI9341 driver + graphics
│  ├─ BeeWiFi.*                # Wi-Fi service
│  └─ BeeNeXTBoard.h           # first board profile
└─ examples/
   ├─ HelloDisplay/
   ├─ WiFiStatus/
   ├─ GoldChart/
   └─ BackgroundFetch/
```

## Examples

### HelloDisplay
Minimal display bring-up using only `Feen.begin()` and `Feen.display()`.

### WiFiStatus
Shows `Feen.connectWiFi()` and automatic reconnect. Replace the placeholder SSID/password locally before flashing. Do not commit private credentials.

### GoldChart
Animated 320x240 gold-price chart demo. It currently uses simulated price data so display and rendering can be tested independently of an external API.

### BackgroundFetch
Runs HTTP/HTTPS GET requests on a dedicated FreeRTOS task while `loop()` stays available for UI and touch work.

```cpp
Feen.fetch().timeout(8000);
Feen.fetch().maxBodyBytes(4096);
Feen.fetch().begin("http://example.com/", 10000);

void loop() {
  Feen.update();

  feen::FetchResult result;
  if (Feen.fetch().read(result)) {
    if (result.ok()) Serial.println(result.body);
    else Serial.println(result.error);
  }
}
```

For HTTPS, use `setCACert()` in production. `setInsecureHttps(true)` exists only for controlled testing. Responses are capped in RAM, and a `truncated` flag is returned if the configured body limit is reached.

## Arduino ESP32 build target

Verified with Arduino ESP32 core **2.0.13** on `ESP32 Dev Module`, 4 MB flash, DIO mode. The physical test board uses a CH340 serial bridge.

Typical display/Wi-Fi examples are about **55% flash / 13% RAM**. The `BackgroundFetch` example, which pulls in HTTP/HTTPS/TLS support, is about **68% flash / 14% RAM** on the current target.

## Security

Wi-Fi secrets and local files are ignored by `.gitignore`. Do not commit full-flash backups or credential files.

## Roadmap

1. Core display + Feen facade — done
2. Wi-Fi service + reconnect — done
3. Background HTTP/HTTPS fetch service — done
4. XPT2046 touch + calibration
5. Small reusable UI widgets (`Button`, `Label`, `Slider`, `Chart`)
6. Simple screen/navigation system
7. JSON helpers + live gold-price data source
8. SD storage and settings
9. OTA and recovery UI
10. Additional Feen board profiles
