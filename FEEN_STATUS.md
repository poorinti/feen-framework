# Feen Framework — Project Status

Updated: 2026-09-08

Repository: https://github.com/poorinti/feen-framework

## Current milestone

**Feen Framework 0.2.0** adds a generic non-blocking background HTTP/HTTPS GET service. BeeNeXT 2.8R remains the first verified board profile.

Feen does not depend on BlynkGO at runtime. BlynkGO source was used only as a reference for BeeNeXT board pin mapping and hardware behavior.

## Verified hardware

- Board: BeeNeXT 2.8R
- MCU: ESP32-D0WD-V3, dual-core 240 MHz
- Flash: 4 MB
- Serial bridge: CH340 / COM4
- LCD: ILI9341, 320x240 landscape
- LCD pins: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch controller: XPT2046 reference pins INT 36, MOSI 32, MISO 39, SCLK 25, CS 33
- SD pins: SCLK 18, MISO 19, MOSI 23, CS 5
- LDR: GPIO34
- Speaker: GPIO26
- RGB LED: GPIO4 / GPIO16 / GPIO17

## Working now

### Framework / display

- `#include <Feen.h>` public API
- `Feen.begin()`
- `Feen.display()`
- `Feen.update()`
- Standalone ILI9341 initialization
- Basic graphics primitives
- Built-in small text rendering
- Correct non-mirrored BeeNeXT 2.8R landscape orientation

### Wi-Fi

- `Feen.connectWiFi(ssid, password)`
- `Feen.wifi()`
- Wi-Fi station mode
- Automatic Wi-Fi reconnect

### Background fetch — new in 0.2.0

- `Feen.fetch()`
- `Feen.fetch().begin(url, intervalMs)`
- `Feen.fetch().fetchNow()`
- `Feen.fetch().available()`
- `Feen.fetch().read(result)`
- configurable request interval
- configurable timeout
- configurable maximum response body size
- up to six custom request headers
- HTTPS CA certificate support through `setCACert()`
- explicit insecure HTTPS test mode through `setInsecureHttps(true)`
- HTTP/HTTPS GET runs on a dedicated low-priority FreeRTOS task pinned to core 0
- UI/main loop reads results through a mutex-protected latest-result mailbox
- background task never draws to the display directly
- response body is memory-capped and reports `truncated=true` if the cap is reached
- interval `0` provides one-shot mode; `fetchNow()` can trigger another request

## Background fetch architecture

The main/UI loop stays on the normal Arduino execution path. Network requests may block internally on DNS, TCP, TLS or the remote server, but that blocking happens inside the `FeenFetch` FreeRTOS task instead of the UI loop.

The task writes only to a protected result mailbox. Application code consumes the newest completed result from `loop()`:

```cpp
feen::FetchResult result;
if (Feen.fetch().read(result)) {
  if (result.ok()) {
    // update application state or UI here, from the main loop
  }
}
```

This avoids cross-core display access and reduces UI/touch race conditions.

## Examples

- `examples/HelloDisplay`
- `examples/WiFiStatus`
- `examples/GoldChart`
- `examples/BackgroundFetch`

`BackgroundFetch` uses placeholder credentials and a public plain-HTTP endpoint so it compiles without embedding certificates or secrets. Production HTTPS should use a trusted CA certificate.

## Verified builds

All four public examples compile successfully with Arduino ESP32 core 2.0.13 on `ESP32 Dev Module`, DIO, 4 MB flash, no PSRAM.

Observed sizes after adding FeenFetch:

- HelloDisplay: about 55% flash / 13% RAM
- WiFiStatus: about 55% flash / 13% RAM
- GoldChart: about 55% flash / 13% RAM
- BackgroundFetch: about 68% flash / 14% RAM

The additional flash use in BackgroundFetch comes mainly from HTTP/HTTPS/TLS support.

## Physical board verification

The Feen 0.2.0 BackgroundFetch smoke test was compiled and flashed to the physical BeeNeXT 2.8R through COM4. Upload hash verification passed.

Runtime test configuration:

- Wi-Fi connected using the ignored local credential file / local smoke sketch
- request interval: 10 seconds
- response cap: 4096 bytes
- UI loop updated a visible `LOOP` counter every 250 ms
- Serial emitted `LOOP_ALIVE` every 1 second

Observed HTTP smoke test:

- endpoint: `http://example.com/`
- repeated `HTTP 200`
- response body: 559 bytes
- `truncated=no`
- `LOOP_ALIVE` continued every second through the request periods

Observed HTTPS smoke test:

- endpoint: GitHub repository API over HTTPS
- TLS test used `setInsecureHttps(true)` only in the ignored local smoke sketch
- custom `User-Agent` request header was accepted
- `HTTP 200`
- response was capped at 4096 bytes and correctly reported `truncated=yes`
- Serial showed `fetch=FETCHING` while `LOOP_ALIVE` and the UI tick counter continued advancing

No watchdog reset or application crash was observed during either smoke-test window. This verifies that both the current HTTP and HTTPS fetch paths run without blocking the main/UI loop, and that the response memory cap works at runtime.

## Safety / recovery

The original 4 MB board flash was backed up before custom firmware work. Keep that local backup outside Git so the factory/application image can be restored if needed.

Do not commit:

- Wi-Fi passwords
- local configuration files
- local smoke-test sketches containing credentials
- full-flash backup images
- build output binaries

The repository `.gitignore` excludes `local/`, example secret headers, build outputs and binary artifacts.

## Git history checkpoints

- GitHub repository: `poorinti/feen-framework`
- Default branch: `main`
- Initial framework commit: `547f1e3` — `Feen Framework 0.1.0`
- Documentation checkpoint: `e8e1b3a` — `Document Feen Framework 0.1.0 status`
- Repository is public
- Local secrets are excluded through `.gitignore`

## Next development order

1. XPT2046 resistive touch driver
2. Touch calibration and coordinate rotation
3. Reusable `Button`, `Label`, `Slider` and `Chart` widgets
4. Simple screen/navigation system
5. JSON parsing/helper layer for fetched API data
6. Real gold-price data source using `Feen.fetch()`
7. SD storage and settings
8. OTA update and recovery screen
9. Additional Feen board profiles

## Design direction

Feen should stay lightweight and easy to use. Application code should remain outside the framework core whenever possible. BeeNeXT is only the first supported board; board-specific pin mapping should remain isolated so future boards can use the same Feen API.

Network work must remain isolated from UI rendering. Background services should communicate back to application code using bounded, thread-safe state rather than drawing directly from worker tasks.
