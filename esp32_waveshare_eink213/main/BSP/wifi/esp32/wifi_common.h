#pragma once

#include "esp_wifi.h"
#include "esp_wifi_types.h"

struct t_wifi_common {
    esp_netif_t *netif = NULL;
};

extern struct t_wifi_common wifi_common;