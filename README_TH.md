# Feen Framework — คู่มือภาษาไทย

Feen Framework เป็นเฟรมเวิร์ก ESP32 แบบเบาสำหรับบอร์ดจอขนาดเล็ก โดยบอร์ดตัวแรกที่ทดสอบจริงคือ **BeeNeXT 2.8R**

เวอร์ชันปัจจุบัน: **0.2.0**

GitHub: https://github.com/poorinti/feen-framework

## ความสามารถที่มีตอนนี้

- ใช้งานผ่าน `#include <Feen.h>`
- ควบคุมจอ ILI9341 320x240
- วาดเส้น สี่เหลี่ยม พื้นหลัง และข้อความแบบพื้นฐาน
- เชื่อม Wi-Fi และ reconnect อัตโนมัติ
- Fetch HTTP/HTTPS แบบ background โดยไม่บล็อก UI/main loop
- ตั้ง interval, timeout และขนาด response สูงสุดได้
- ใส่ HTTP headers ได้สูงสุด 6 รายการ
- รองรับ HTTPS แบบ CA certificate
- มีโหมด HTTPS แบบ insecure สำหรับทดสอบเท่านั้น
- มีตัวอย่าง `HelloDisplay`, `WiFiStatus`, `GoldChart` และ `BackgroundFetch`

## ฮาร์ดแวร์ที่ทดสอบแล้ว

BeeNeXT 2.8R:

- MCU: ESP32-D0WD-V3
- Flash: 4 MB
- จอ: ILI9341 320x240
- USB Serial: CH340
- Arduino ESP32 Core ที่ใช้ทดสอบ: **2.0.13**

ค่าบอร์ดที่ใช้ทดสอบใน Arduino:

- Board: `ESP32 Dev Module`
- Flash Size: `4MB`
- Flash Mode: `DIO`
- Partition Scheme: `Default`
- PSRAM: `Disabled / No PSRAM`

## วิธีติดตั้ง

### วิธีที่ 1: Download ZIP จาก GitHub

1. เปิด https://github.com/poorinti/feen-framework
2. กด `Code` > `Download ZIP`
3. เปิด Arduino IDE
4. ไปที่ `Sketch` > `Include Library` > `Add .ZIP Library...`
5. เลือกไฟล์ ZIP ที่ดาวน์โหลดมา
6. เปิดตัวอย่างจากเมนู Examples ของ Arduino IDE หรือสร้าง sketch ใหม่แล้วใส่ `#include <Feen.h>`

### วิธีที่ 2: Clone ด้วย Git

Clone repo ไปไว้ในโฟลเดอร์ libraries ของ Arduino เช่น:

```bash
git clone https://github.com/poorinti/feen-framework.git
```

จากนั้นปิดและเปิด Arduino IDE ใหม่หนึ่งครั้ง

## เริ่มต้นใช้งานจอ

ตัวอย่างขั้นต่ำ:

```cpp
#include <Feen.h>

void setup() {
  Serial.begin(115200);

  if (!Feen.begin()) {
    Serial.println("Feen init failed");
    return;
  }

  auto& lcd = Feen.display();
  lcd.fillScreen(feen::color::BLACK);
  lcd.drawText(20, 20, "FEEN", feen::color::GOLD, feen::color::BLACK, 3);
}

void loop() {
  Feen.update();
}
```

สิ่งสำคัญคือให้เรียก `Feen.update()` ใน `loop()` อยู่เสมอ เพื่อให้ service ต่าง ๆ ของ Feen ทำงานต่อเนื่อง

## เชื่อม Wi-Fi

```cpp
#include <Feen.h>

static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);
  Feen.begin();
  Feen.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  Feen.update();

  if (Feen.wifi().connected()) {
    Serial.println(Feen.wifi().ip());
  }

  delay(1000);
}
```

Feen มี reconnect logic ในตัว แต่ application ยังคงต้องเรียก `Feen.update()` เป็นประจำ

## Fetch API แบบ background

Feen 0.2.0 สามารถยิง HTTP/HTTPS GET ใน FreeRTOS task แยกบน Core 0 ทำให้ `loop()` หลักยังใช้วาดจอ อ่าน touch หรือทำ logic อื่นได้ต่อเนื่อง

ตัวอย่างดึงข้อมูลทุก 10 วินาที:

```cpp
#include <Feen.h>

static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* API_URL = "http://example.com/";

void setup() {
  Serial.begin(115200);
  Feen.begin();
  Feen.connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  Feen.fetch().timeout(8000);
  Feen.fetch().maxBodyBytes(4096);
  Feen.fetch().begin(API_URL, 10000);
}

void loop() {
  Feen.update();

  feen::FetchResult result;
  if (Feen.fetch().read(result)) {
    Serial.printf("HTTP %d\n", result.statusCode);

    if (result.ok()) {
      Serial.println(result.body);
    } else {
      Serial.println(result.error);
    }

    if (result.truncated) {
      Serial.println("response ถูกตัดตาม maxBodyBytes");
    }
  }

  // วาด UI / อ่าน touch / ทำงานอื่นต่อได้ตรงนี้
  delay(2);
}
```

## ตั้งรอบการ Fetch

เริ่มด้วย interval:

```cpp
Feen.fetch().begin("http://example.com/", 30000); // ทุก 30 วินาที
```

เปลี่ยน interval ภายหลัง:

```cpp
Feen.fetch().interval(15000); // ทุก 15 วินาที
```

ถ้าต้องการ one-shot ให้ใช้ interval `0`:

```cpp
Feen.fetch().begin("http://example.com/", 0);
```

สั่ง fetch เพิ่มทันทีโดยไม่รอรอบถัดไป:

```cpp
Feen.fetch().fetchNow();
```

หยุด service:

```cpp
Feen.fetch().stop();
```

## ตรวจสถานะ Fetch

```cpp
Serial.println(Feen.fetch().stateText());
```

สถานะภายในมี เช่น:

- `Idle`
- `WaitingForWiFi`
- `Fetching`
- `Ready`
- `Error`
- `Stopped`

ผลล่าสุดจะอ่านได้ด้วย:

```cpp
if (Feen.fetch().available()) {
  feen::FetchResult result;
  if (Feen.fetch().read(result)) {
    // ใช้ result ได้ตรงนี้
  }
}
```

`read()` จะ consume ผลที่ยังไม่ได้อ่านใน mailbox ล่าสุด

## จำกัดขนาด Response เพื่อประหยัด RAM

ESP32 มี RAM จำกัด จึงควรกำหนดขนาด response สูงสุด โดยเฉพาะ API ที่ตอบ JSON ใหญ่

```cpp
Feen.fetch().maxBodyBytes(4096);
```

ถ้า response ใหญ่เกิน limit:

```cpp
result.truncated == true
```

ตัว Feen จะเก็บข้อมูลเท่าที่กำหนด ไม่ปล่อยให้ response โตแบบไม่จำกัด

## ตั้ง Timeout

```cpp
Feen.fetch().timeout(8000); // 8 วินาที
```

Network request อาจรอ DNS, TCP, TLS หรือ server ได้ แต่การรอนั้นเกิดใน background task ไม่ใช่ใน UI loop

## เพิ่ม HTTP Header

เพิ่มได้สูงสุด 6 รายการ:

```cpp
Feen.fetch().addHeader("Accept", "application/json");
Feen.fetch().addHeader("User-Agent", "Feen/0.2.0");
```

ล้าง header ทั้งหมด:

```cpp
Feen.fetch().clearHeaders();
```

อย่า hard-code API key หรือ token จริงลงไฟล์ที่จะ push ขึ้น GitHub

## HTTPS แบบปลอดภัย

สำหรับใช้งานจริงควรตั้ง CA certificate:

```cpp
static const char* ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
...CA CERTIFICATE...
-----END CERTIFICATE-----
)EOF";

Feen.fetch().setCACert(ROOT_CA);
Feen.fetch().begin("https://api.example.com/data", 15000);
```

ถ้าเป็นการทดสอบชั่วคราวบนเครือข่ายที่ไว้ใจได้ สามารถใช้:

```cpp
Feen.fetch().setInsecureHttps(true);
Feen.fetch().begin("https://api.example.com/data", 15000);
```

**ไม่แนะนำ `setInsecureHttps(true)` สำหรับ production** เพราะจะไม่ตรวจสอบ certificate ของ server

## อ่านผลจาก FetchResult

ตัวแปร `feen::FetchResult` มีข้อมูลหลักดังนี้:

```cpp
result.statusCode   // HTTP status เช่น 200, 404, 500
result.body         // response body
result.error        // ข้อความ error
result.completedAt  // เวลา millis ตอน request เสร็จ
result.truncated    // true ถ้า body ถูกตัดตาม maxBodyBytes
result.ok()         // true เมื่อ status อยู่ในช่วง 200-299
```

## ทำไม Fetch ถึงไม่ทำให้จอค้าง

Feen ออกแบบให้ request ทำงานใน FreeRTOS worker task แยกจาก Arduino main loop โดย worker จะไม่วาดจอโดยตรง

ผลลัพธ์ถูกส่งกลับผ่าน mailbox ที่มี mutex ป้องกัน จากนั้น application ค่อยอ่านผลใน `loop()` และอัปเดต UI เอง

แนวทางที่แนะนำ:

```text
FreeRTOS Background Task
        |
        | HTTP / HTTPS
        v
Thread-safe Result Mailbox
        |
        v
Arduino loop()
        |
        +--> Update UI
        +--> Update Chart
        +--> Touch / Buttons
```

บน BeeNeXT 2.8R ได้ทดสอบทั้ง HTTP และ HTTPS จริงแล้ว ระหว่างสถานะ `FETCHING` ตัว main loop ยังเดินต่อเนื่อง

## ตัวอย่างโครงสำหรับ API ราคาทอง

ตอนนี้ Feen มี background fetch แล้ว ขั้นต่อไปสามารถนำ JSON จาก API ราคาทองมาวิเคราะห์และป้อนเข้ากราฟได้ เช่น:

```cpp
void loop() {
  Feen.update();

  feen::FetchResult result;
  if (Feen.fetch().read(result) && result.ok()) {
    // 1. parse JSON จาก result.body
    // 2. อ่านราคาทอง
    // 3. เก็บค่าล่าสุด
    // 4. update กราฟจาก main/UI loop
  }
}
```

อย่า parse หรือวาดจอจาก background worker โดยตรง ให้ทำใน `loop()` หลัง `read()` เพื่อให้ UI ปลอดภัยและดูแลง่าย

## ตัวอย่างที่มีใน Repository

- `examples/HelloDisplay` — เริ่มต้นจอ
- `examples/WiFiStatus` — เชื่อม Wi-Fi
- `examples/GoldChart` — กราฟทองแบบข้อมูลจำลอง
- `examples/BackgroundFetch` — HTTP/HTTPS background fetch

## เรื่องรหัสผ่านและ Secret

Repository นี้เป็น Public ดังนั้น:

- อย่า commit รหัส Wi-Fi จริง
- อย่า commit API key / token
- อย่า commit full-flash backup
- ใช้ placeholder เช่น `YOUR_WIFI_SSID` และ `YOUR_WIFI_PASSWORD` ใน examples
- เก็บไฟล์ local/private ไว้ในโฟลเดอร์ที่ `.gitignore` ครอบคลุม

## ขนาดโปรแกรมโดยประมาณ

จาก build ที่ทดสอบกับ ESP32 Arduino Core 2.0.13:

- HelloDisplay: ประมาณ 55% Flash / 13% RAM
- WiFiStatus: ประมาณ 55% Flash / 13% RAM
- GoldChart: ประมาณ 55% Flash / 13% RAM
- BackgroundFetch: ประมาณ 68% Flash / 14% RAM

BackgroundFetch ใช้ Flash เพิ่มเพราะมี HTTP/HTTPS/TLS stack

## แผนงานต่อ

1. XPT2046 Touch + calibration
2. Widget เบื้องต้น เช่น `Button`, `Label`, `Slider`, `Chart`
3. Screen/navigation system
4. JSON helper
5. API ราคาทองจริง + live chart
6. SD settings/logging
7. OTA + recovery
8. รองรับบอร์ด Feen เพิ่มเติม

ดูสถานะพัฒนาล่าสุดได้ที่ [`FEEN_STATUS.md`](FEEN_STATUS.md)
