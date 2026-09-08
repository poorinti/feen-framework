# Feen Framework — Project Status

Updated: 2026-09-08

Repository: https://github.com/poorinti/feen-framework

## Current milestone

**Feen Framework 0.1.0** is working as a reusable Arduino ESP32 library. BeeNeXT 2.8R is the first verified board profile.

The framework no longer depends on BlynkGO at runtime. BlynkGO source was used only as a reference for the BeeNeXT board pin mapping and hardware behavior.

## Verified hardware

- Board: BeeNeXT 2.8R
- MCU: ESP32-D0WD-V3, dual-core 240 MHz
- Flash: 4 MB
- Serial bridge: CH340
- LCD: ILI9341, 320x240 landscape
- LCD pins: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch controller: XPT2046 reference pins INT 36, MOSI 32, MISO 39, SCLK 25, CS 33
- SD pins: SCLK 18, MISO 19, MOSI 23, CS 5
- LDR: GPIO34
- Speaker: GPIO26
- RGB LED: GPIO4 / GPIO16 / GPIO17

## Working now

- `#include <Feen.h>` public API
- `Feen.begin()`
- `Feen.display()`
- `Feen.connectWiFi(ssid, password)`
- `Feen.wifi()`
- `Feen.update()`
- Standalone ILI9341 initialization
- Basic graphics primitives
- Built-in small text rendering
- Wi-Fi station mode
- Automatic Wi-Fi reconnect
- Correct non-mirrored BeeNeXT 2.8R landscape orientation
- Gold chart demo
- HelloDisplay example
- WiFiStatus example

## Verified builds

All current examples compile successfully with Arduino ESP32 core 2.0.13:

- `examples/HelloDisplay`
- `examples/WiFiStatus`
- `examples/GoldChart`

Typical current build size is about 54% flash and 12–13% RAM on the ESP32 Dev Module 4 MB target.

## Physical board verification

The custom firmware was flashed to the physical BeeNeXT 2.8R through COM4. Flash hash verification passed and the firmware booted successfully.

The gold chart demo was verified running with simulated values around 51,xxx THB. A previous 16-bit price overflow bug was fixed by moving price/sample storage to 32-bit values.

Wi-Fi was also verified on the physical board. Credentials are stored only in ignored local files and are not committed to Git.

## Safety / recovery

The original 4 MB board flash was backed up before custom firmware work. Keep the local backup outside Git so the factory/application image can be restored if needed.

Do not commit:

- Wi-Fi passwords
- local configuration files
- full-flash backup images
- build output binaries

## Git status at milestone 0.1.0

- GitHub repository: `poorinti/feen-framework`
- Default branch: `main`
- Initial framework commit: `547f1e3` — `Feen Framework 0.1.0`
- Repository is public
- Local secrets are excluded through `.gitignore`

## Next development order

1. XPT2046 resistive touch driver
2. Touch calibration and coordinate rotation
3. Reusable `Button`, `Label`, `Slider` and `Chart` widgets
4. Simple screen/navigation system
5. HTTPS + JSON client for live data
6. Real gold-price data source
7. SD storage and settings
8. OTA update and recovery screen
9. Additional Feen board profiles

## Design direction

Feen should stay lightweight and easy to use. Application code should remain outside the framework core whenever possible. BeeNeXT is only the first supported board; board-specific pin mapping should remain isolated so future boards can use the same Feen API.
