#pragma once

#include "BSP/display/EPD_2in13_V4/display.h"
#include "BSP/gpio/esp32_waveshare_eink213/gpio.h"
#include "BSP/network/esp32/http_server.h"
#include "BSP/network/esp32/ip4_addr.h"
#include "BSP/network/esp32/mac.h"
#include "BSP/network/esp32/smartconfig.h"
#include "BSP/network/esp32/sntp_client.h"
#include "BSP/storage/esp32/flash.h"
#include "BSP/storage/esp32/memory.h"
#include "BSP/storage/esp32/persistent_storage.h"
#include "BSP/system/esp32/attr.h"
#include "BSP/system/esp32/ota.h"
#include "BSP/wifi/esp32/wifi.h"
#include "BSP/wifi/esp32/wifi_common.h"
#include "Utils/lassert.h"
#include "config.h"

struct t_board {
    event_t gpio_events;
    event_t display_events;
};

t_board board_init();
void print_board_info();

void restart_system();