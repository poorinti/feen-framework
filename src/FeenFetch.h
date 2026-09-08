#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace feen {

enum class FetchState : uint8_t {
  Idle,
  WaitingForWiFi,
  Fetching,
  Ready,
  Error,
  Stopped
};

struct FetchResult {
  int statusCode = 0;
  String body;
  String error;
  uint32_t completedAt = 0;
  bool truncated = false;

  bool ok() const { return statusCode >= 200 && statusCode < 300; }
};

class BackgroundFetch {
public:
  BackgroundFetch();

  // Start or reconfigure a periodic GET request. intervalMs == 0 means one-shot.
  bool begin(const char* url, uint32_t intervalMs = 30000);
  void stop();

  void interval(uint32_t intervalMs);
  void timeout(uint16_t timeoutMs);
  void maxBodyBytes(size_t maxBytes);

  // HTTPS is secure by default. Use a CA certificate in production.
  void setCACert(const char* caCert);
  void setInsecureHttps(bool enabled = true);

  // Up to six request headers are retained and copied into each request.
  bool addHeader(const char* name, const char* value);
  void clearHeaders();

  // Schedule an immediate request without blocking the caller.
  void fetchNow();

  // Latest-result mailbox. read() consumes the current unread result.
  bool available() const;
  bool read(FetchResult& out);

  bool running() const;
  FetchState state() const;
  const char* stateText() const;

private:
  static constexpr uint8_t MAX_HEADERS = 6;

  struct RequestConfig {
    String url;
    String caCert;
    String headerNames[MAX_HEADERS];
    String headerValues[MAX_HEADERS];
    uint8_t headerCount = 0;
    uint32_t intervalMs = 30000;
    uint16_t timeoutMs = 8000;
    size_t maxBodyBytes = 16384;
    bool insecureHttps = false;
  };

  mutable SemaphoreHandle_t mutex_ = nullptr;
  TaskHandle_t task_ = nullptr;
  RequestConfig config_;
  FetchResult result_;
  FetchState state_ = FetchState::Idle;
  uint32_t resultVersion_ = 0;
  uint32_t consumedVersion_ = 0;
  uint32_t lastFetchAt_ = 0;
  bool enabled_ = false;
  bool forceFetch_ = false;

  bool ensureMutex() const;
  void setState(FetchState state);
  void publishResult(int statusCode, const String& body, const String& error,
                     bool truncated);
  bool copyConfig(RequestConfig& out, bool& enabled, bool& forced,
                  uint32_t& lastFetchAt);
  void performRequest(const RequestConfig& config);
  void taskLoop();
  static void taskEntry(void* arg);
};

} // namespace feen
