#pragma once

#include "esp_err.h"
#include "esp_log.h"

#define log_e(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define log_w(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#define log_i(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define log_d(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)

typedef enum {
    LOG_NONE = esp_log_level_t::ESP_LOG_NONE,
    LOG_ERROR = esp_log_level_t::ESP_LOG_ERROR,
    LOG_WARN = esp_log_level_t::ESP_LOG_WARN,
    LOG_INFO = esp_log_level_t::ESP_LOG_INFO,
    LOG_DEBUG = esp_log_level_t::ESP_LOG_DEBUG,
} log_level_t;

static inline void log_level_set(const char *tag, log_level_t level) {
    esp_log_level_set(tag, (esp_log_level_t)level);
}