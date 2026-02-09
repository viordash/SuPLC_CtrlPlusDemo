

#include "board.h"
#include "os.h"

static const char *TAG_wifi_sta = "wifi_sta";

static struct {
    wifi_sta_connect_event_handler_t connect_event_handler;
    wifi_sta_fail_event_handler_t fail_event_handler;
} wifi_sta;

static void
wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void
ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void wifi_start_station(wifi_station_config_t *config) {
    log_i(TAG_wifi_sta, "start, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif == NULL ? STATUS_OK : STATUS_FAIL);

    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, config->ssid,
           sizeof(wifi_config.sta.ssid)); //
    memcpy(wifi_config.sta.password, config->password, sizeof(wifi_config.sta.password));
    if (wifi_config.sta.password[0] != 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    wifi_sta.connect_event_handler = config->connect_event_handler;
    wifi_sta.fail_event_handler = config->fail_event_handler;

    ERROR_CHECK(esp_netif_init());
    ERROR_CHECK(esp_event_loop_create_default());

    wifi_common.netif = esp_netif_create_default_wifi_sta();
    assert(wifi_common.netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ERROR_CHECK(esp_wifi_init(&cfg));
    ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                           ESP_EVENT_ANY_ID,
                                           wifi_event_handler,
                                           config->event_handler_arg));
    ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                           IP_EVENT_STA_GOT_IP,
                                           ip_event_handler,
                                           config->event_handler_arg));

    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ERROR_CHECK(esp_wifi_start());
}

void wifi_stop_station() {
    log_i(TAG_wifi_sta, "stop, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif != NULL ? STATUS_OK : STATUS_FAIL);
    esp_wifi_disconnect();
    esp_wifi_stop();

    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler);

    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(wifi_common.netif);
    esp_event_loop_delete_default();
    esp_netif_deinit();
    wifi_common.netif = NULL;
}

bool get_wifi_station_rssi(int8_t *rssi) {
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) != STATUS_OK) {
        log_i(TAG_wifi_sta, "rssi, no connection");
        *rssi = INT8_MIN;
        return false;
    }
    *rssi = ap.rssi;
    return true;
}

void get_wifi_station_creds(char *ssid, size_t ssid_size, char *password, size_t password_size) {
    wifi_config_t wifi_config = {};
    ERROR_CHECK(esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_config));
    strncpy(ssid, (char *)wifi_config.sta.ssid, ssid_size);
    ssid[ssid_size - 1] = 0;
    strncpy(password, (char *)wifi_config.sta.password, password_size);
    password[password_size - 1] = 0;
}

static void
wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)event_data;
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            log_d(TAG_wifi_sta, "start wifi_sta event");
            esp_wifi_connect();
            return;

        case WIFI_EVENT_STA_DISCONNECTED:
            log_d(TAG_wifi_sta, "connect to the AP fail");
            wifi_sta.fail_event_handler(arg);
            return;

        case WIFI_EVENT_STA_STOP:
            log_d(TAG_wifi_sta, "stop wifi_sta event");
            return;

        case WIFI_EVENT_STA_CONNECTED:
            log_d(TAG_wifi_sta, "wifi_sta connected event");
            return;

        case WIFI_EVENT_HOME_CHANNEL_CHANGE:
            log_d(TAG_wifi_sta, "wifi_sta home channel change");
            return;

        default:
            log_w(TAG_wifi_sta,
                  "unhandled event, event_base:%s, event_id:%d",
                  event_base,
                  (int)event_id);
            break;
    }
}

static void
ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)event_base;
    (void)event_id;

    auto event = (ip_event_got_ip_t *)event_data;
    log_d(TAG_wifi_sta, "got ip:%s", ip4addr_ntoa((const ip4_addr_t *)&event->ip_info.ip));
    wifi_sta.connect_event_handler(arg);
}
