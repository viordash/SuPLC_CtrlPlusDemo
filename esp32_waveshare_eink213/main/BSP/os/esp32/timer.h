#pragma once

#include "esp_timer.h"

#define timer_get_time_us() (uint64_t)esp_timer_get_time()