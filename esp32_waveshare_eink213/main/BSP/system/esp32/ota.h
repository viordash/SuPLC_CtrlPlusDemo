#pragma once

#include "esp_ota_ops.h"

typedef esp_ota_handle_t ota_handle_t;

#define ota_get_boot_partition() esp_ota_get_boot_partition()
#define ota_get_running_partition() esp_ota_get_running_partition()
#define ota_get_next_update_partition(start_from) esp_ota_get_next_update_partition(start_from)
#define ota_begin(partition, image_size, out_handle)                                               \
    esp_ota_begin(partition, image_size, out_handle)
#define ota_end(handle) esp_ota_end(handle)
#define ota_write(handle, data, size) esp_ota_write(handle, data, size)
#define ota_set_boot_partition(partition) esp_ota_set_boot_partition(partition)