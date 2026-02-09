#pragma once

#include "esp_flash.h"
#include "nvs_flash.h"

#define get_flash_size(out_size) esp_flash_get_size(NULL, out_size)

typedef esp_partition_t partition_t;