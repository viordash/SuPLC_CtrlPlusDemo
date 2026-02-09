#include "BSP/system/esp32/chip_info.h"
#include "board.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

static void system_init() {
    error_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ERROR_CHECK(ret);
    log_level_set("wifi", LOG_WARN);
    log_level_set("wifi_init", LOG_WARN);
}

t_board board_init() {
    t_board board = {};
    system_init();
    board.gpio_events = gpio_init();
    board.display_events = display_hw_config();
    return board;
}

void print_board_info() {
    /* Print chip information */
    chip_info_t chip_info;
    uint32_t flash_size;
    get_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    int major_rev = chip_info.revision / 100;
    int minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (get_flash_size(&flash_size) != STATUS_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n",
           flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", get_minimum_free_heap_size());
}

void restart_system() {
    esp_restart();
}