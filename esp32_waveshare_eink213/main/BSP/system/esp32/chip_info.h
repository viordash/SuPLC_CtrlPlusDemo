#pragma once

#include "esp_chip_info.h"

#define chip_info_t esp_chip_info_t
#define get_chip_info(out_info) esp_chip_info(out_info)