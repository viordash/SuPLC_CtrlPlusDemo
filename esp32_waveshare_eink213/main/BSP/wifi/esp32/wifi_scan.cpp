

#include "board.h"
#include "os.h"

static const char *TAG_wifi_scan = "wifi_scan";

void wifi_start_scan(const char *ssid, uint16_t per_channel_scan_time_ms) {
    log_i(TAG_wifi_scan, "start, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif == NULL ? STATUS_OK : STATUS_FAIL);

    ERROR_CHECK(esp_netif_init());
    ERROR_CHECK(esp_event_loop_create_default());

    wifi_common.netif = esp_netif_create_default_wifi_sta();
    assert(wifi_common.netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ERROR_CHECK(esp_wifi_init(&cfg));
    ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ERROR_CHECK(esp_wifi_start());

    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;
    scan_config.ssid = (uint8_t *)ssid;
    scan_config.scan_type = wifi_scan_type_t::WIFI_SCAN_TYPE_PASSIVE;
    scan_config.scan_time.passive = per_channel_scan_time_ms;

    ERROR_CHECK(esp_wifi_scan_start(&scan_config, false));
}

void wifi_stop_scan() {
    log_i(TAG_wifi_scan, "stop, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif != NULL ? STATUS_OK : STATUS_FAIL);
    esp_wifi_scan_stop();

    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    esp_netif_destroy_default_wifi(wifi_common.netif);
    esp_event_loop_delete_default();
    esp_netif_deinit();
    wifi_common.netif = NULL;
}

error_t wifi_scan_get_records(uint16_t *number, wifi_scan_record_t *ap_records) {
    wifi_ap_record_t ap_info[*number] = {};
    esp_err_t err = esp_wifi_scan_get_ap_records(number, ap_info);
    if (err != STATUS_OK) {
        return (error_t)err;
    }

    for (int i = 0; i < *number; i++) {
        memcpy(ap_records[i].bssid, ap_info[i].bssid, sizeof(ap_records[i].bssid));
        memcpy(ap_records[i].ssid, ap_info[i].ssid, sizeof(ap_records[i].ssid));
        ap_records[i].primary = ap_info[i].primary;
        ap_records[i].rssi = ap_info[i].rssi;
    }

    return STATUS_OK;
}