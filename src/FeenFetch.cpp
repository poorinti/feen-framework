#include "FeenFetch.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

namespace feen {

namespace {

class BoundedBodyStream : public Stream {
public:
  explicit BoundedBodyStream(size_t limit) : limit_(limit) {
    body_.reserve(limit_ < 1024 ? limit_ : 1024);
  }

  size_t write(uint8_t value) override {
    if (body_.length() < limit_) {
      body_ += static_cast<char>(value);
    } else {
      truncated_ = true;
    }
    return 1;
  }

  size_t write(const uint8_t* buffer, size_t size) override {
    const size_t used = body_.length();
    const size_t room = used < limit_ ? limit_ - used : 0;
    const size_t keep = size < room ? size : room;
    if (keep > 0) body_.concat(reinterpret_cast<const char*>(buffer), keep);
    if (keep < size) truncated_ = true;
    // Report all bytes consumed so HTTPClient can finish/drain the response
    // without retaining an unbounded body in RAM.
    return size;
  }

  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}

  const String& body() const { return body_; }
  bool truncated() const { return truncated_; }

private:
  size_t limit_;
  String body_;
  bool truncated_ = false;
};

} // namespace

BackgroundFetch::BackgroundFetch() = default;

bool BackgroundFetch::ensureMutex() const {
  if (mutex_) return true;
  // Creation is serialized in normal Arduino use (setup/main task). If two
  // callers race before creation, the extra mutex is harmlessly leaked only
  // during that unsupported startup race; normal calls are protected after it.
  mutex_ = xSemaphoreCreateMutex();
  return mutex_ != nullptr;
}

bool BackgroundFetch::begin(const char* url, uint32_t intervalMs) {
  if (!url || !*url || !ensureMutex()) return false;

  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.url = url;
  config_.intervalMs = intervalMs;
  enabled_ = true;
  forceFetch_ = true;
  lastFetchAt_ = 0;
  state_ = FetchState::Idle;
  xSemaphoreGive(mutex_);

  if (!task_) {
    BaseType_t created = xTaskCreatePinnedToCore(
      taskEntry,
      "FeenFetch",
      8192,
      this,
      1,
      &task_,
      0
    );
    if (created != pdPASS) {
      xSemaphoreTake(mutex_, portMAX_DELAY);
      enabled_ = false;
      state_ = FetchState::Error;
      result_.statusCode = -1000;
      result_.error = "Failed to create FeenFetch task";
      result_.completedAt = millis();
      ++resultVersion_;
      xSemaphoreGive(mutex_);
      task_ = nullptr;
      return false;
    }
  }

  return true;
}

void BackgroundFetch::stop() {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  enabled_ = false;
  forceFetch_ = false;
  state_ = FetchState::Stopped;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::interval(uint32_t intervalMs) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.intervalMs = intervalMs;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::timeout(uint16_t timeoutMs) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.timeoutMs = timeoutMs < 250 ? 250 : timeoutMs;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::maxBodyBytes(size_t maxBytes) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.maxBodyBytes = maxBytes < 256 ? 256 : maxBytes;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::setCACert(const char* caCert) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.caCert = caCert ? caCert : "";
  if (caCert && *caCert) config_.insecureHttps = false;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::setInsecureHttps(bool enabled) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  config_.insecureHttps = enabled;
  xSemaphoreGive(mutex_);
}

bool BackgroundFetch::addHeader(const char* name, const char* value) {
  if (!name || !*name || !value || !ensureMutex()) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);

  for (uint8_t i = 0; i < config_.headerCount; ++i) {
    if (config_.headerNames[i].equalsIgnoreCase(name)) {
      config_.headerValues[i] = value;
      xSemaphoreGive(mutex_);
      return true;
    }
  }

  if (config_.headerCount >= MAX_HEADERS) {
    xSemaphoreGive(mutex_);
    return false;
  }

  const uint8_t i = config_.headerCount++;
  config_.headerNames[i] = name;
  config_.headerValues[i] = value;
  xSemaphoreGive(mutex_);
  return true;
}

void BackgroundFetch::clearHeaders() {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  for (uint8_t i = 0; i < MAX_HEADERS; ++i) {
    config_.headerNames[i] = "";
    config_.headerValues[i] = "";
  }
  config_.headerCount = 0;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::fetchNow() {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  if (enabled_) forceFetch_ = true;
  xSemaphoreGive(mutex_);
}

bool BackgroundFetch::available() const {
  if (!ensureMutex()) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  const bool hasResult = resultVersion_ != consumedVersion_;
  xSemaphoreGive(mutex_);
  return hasResult;
}

bool BackgroundFetch::read(FetchResult& out) {
  if (!ensureMutex()) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  if (resultVersion_ == consumedVersion_) {
    xSemaphoreGive(mutex_);
    return false;
  }
  out = result_;
  consumedVersion_ = resultVersion_;
  xSemaphoreGive(mutex_);
  return true;
}

bool BackgroundFetch::running() const {
  if (!ensureMutex()) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  const bool value = enabled_;
  xSemaphoreGive(mutex_);
  return value;
}

FetchState BackgroundFetch::state() const {
  if (!ensureMutex()) return FetchState::Error;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  const FetchState value = state_;
  xSemaphoreGive(mutex_);
  return value;
}

const char* BackgroundFetch::stateText() const {
  switch (state()) {
    case FetchState::Idle: return "IDLE";
    case FetchState::WaitingForWiFi: return "WAIT_WIFI";
    case FetchState::Fetching: return "FETCHING";
    case FetchState::Ready: return "READY";
    case FetchState::Error: return "ERROR";
    case FetchState::Stopped: return "STOPPED";
    default: return "UNKNOWN";
  }
}

void BackgroundFetch::setState(FetchState state) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  state_ = state;
  xSemaphoreGive(mutex_);
}

void BackgroundFetch::publishResult(int statusCode, const String& body,
                                    const String& error, bool truncated) {
  if (!ensureMutex()) return;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  result_.statusCode = statusCode;
  result_.body = body;
  result_.error = error;
  result_.completedAt = millis();
  result_.truncated = truncated;
  ++resultVersion_;
  lastFetchAt_ = result_.completedAt;
  forceFetch_ = false;
  state_ = (statusCode >= 200 && statusCode < 300)
             ? FetchState::Ready
             : FetchState::Error;
  xSemaphoreGive(mutex_);
}

bool BackgroundFetch::copyConfig(RequestConfig& out, bool& enabled,
                                 bool& forced, uint32_t& lastFetchAt) {
  if (!ensureMutex()) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  out = config_;
  enabled = enabled_;
  forced = forceFetch_;
  lastFetchAt = lastFetchAt_;
  xSemaphoreGive(mutex_);
  return true;
}

void BackgroundFetch::performRequest(const RequestConfig& config) {
  if (WiFi.status() != WL_CONNECTED) {
    setState(FetchState::WaitingForWiFi);
    return;
  }

  setState(FetchState::Fetching);
  HTTPClient http;
  http.setTimeout(config.timeoutMs);

  const bool https = config.url.startsWith("https://");
  bool begun = false;

  WiFiClient plainClient;
  WiFiClientSecure secureClient;

  if (https) {
    if (config.insecureHttps) {
      secureClient.setInsecure();
    } else if (config.caCert.length() > 0) {
      secureClient.setCACert(config.caCert.c_str());
    } else {
      publishResult(-1001, "", "HTTPS requires setCACert() or setInsecureHttps(true)", false);
      return;
    }
    begun = http.begin(secureClient, config.url);
  } else {
    begun = http.begin(plainClient, config.url);
  }

  if (!begun) {
    publishResult(-1002, "", "HTTPClient begin failed", false);
    return;
  }

  for (uint8_t i = 0; i < config.headerCount; ++i) {
    http.addHeader(config.headerNames[i], config.headerValues[i]);
  }

  const int statusCode = http.GET();
  if (statusCode <= 0) {
    const String error = HTTPClient::errorToString(statusCode);
    http.end();
    publishResult(statusCode, "", error, false);
    return;
  }

  BoundedBodyStream sink(config.maxBodyBytes);
  const int written = http.writeToStream(&sink);
  if (written < 0) {
    const String error = HTTPClient::errorToString(written);
    http.end();
    publishResult(written, sink.body(), error, sink.truncated());
    return;
  }

  http.end();
  publishResult(statusCode, sink.body(), "", sink.truncated());
}

void BackgroundFetch::taskLoop() {
  for (;;) {
    RequestConfig config;
    bool enabled = false;
    bool forced = false;
    uint32_t lastFetchAt = 0;

    if (!copyConfig(config, enabled, forced, lastFetchAt)) {
      vTaskDelay(pdMS_TO_TICKS(250));
      continue;
    }

    if (!enabled) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (WiFi.status() != WL_CONNECTED) {
      setState(FetchState::WaitingForWiFi);
      vTaskDelay(pdMS_TO_TICKS(250));
      continue;
    }

    const uint32_t now = millis();
    const bool firstRun = lastFetchAt == 0;
    const bool intervalDue = config.intervalMs > 0 &&
                             uint32_t(now - lastFetchAt) >= config.intervalMs;

    if (forced || firstRun || intervalDue) {
      performRequest(config);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void BackgroundFetch::taskEntry(void* arg) {
  static_cast<BackgroundFetch*>(arg)->taskLoop();
}

} // namespace feen
