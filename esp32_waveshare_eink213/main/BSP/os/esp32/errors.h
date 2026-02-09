#pragma once

#include "BSP/os/esp32/system.h"

typedef esp_err_t error_t;

#define ERROR_CHECK ESP_ERROR_CHECK

/* Definitions for error constants. */
#define STATUS_OK ESP_OK
#define STATUS_FAIL ESP_FAIL

#define ERR_NO_MEM ESP_ERR_NO_MEM
#define ERR_INVALID_ARG ESP_ERR_INVALID_ARG
#define ERR_INVALID_STATE ESP_ERR_INVALID_STATE