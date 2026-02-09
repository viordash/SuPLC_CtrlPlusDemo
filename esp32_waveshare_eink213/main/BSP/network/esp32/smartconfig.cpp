

#include "board.h"
#include "esp_smartconfig.h"
#include "os.h"

static const char *TAG_smartconfig = "drivers/smartconfig";

static struct {
    smartconfig_started_event_handler_t started_event_handler;
    smartconfig_disconnected_event_handler_t disconnected_event_handler;
    smartconfig_connected_event_handler_t connected_event_handler;
    smartconfig_scan_done_event_handler_t scan_done_event_handler;
    smartconfig_found_event_handler_t found_event_handler;
    smartconfig_got_creds_event_handler_t got_creds_event_handler;
    smartconfig_done_event_handler_t done_event_handler;
} smartconfig;

static void
wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void
ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void
sc_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void start_smartconfig(smartconfig_config_t *config) {
    log_i(TAG_smartconfig, "start, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif == NULL ? STATUS_OK : STATUS_FAIL);

    smartconfig.started_event_handler = config->started_event_handler;
    smartconfig.disconnected_event_handler = config->disconnected_event_handler;
    smartconfig.connected_event_handler = config->connected_event_handler;
    smartconfig.scan_done_event_handler = config->scan_done_event_handler;
    smartconfig.found_event_handler = config->found_event_handler;
    smartconfig.got_creds_event_handler = config->got_creds_event_handler;
    smartconfig.done_event_handler = config->done_event_handler;

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
    ERROR_CHECK(esp_event_handler_register(SC_EVENT,
                                           ESP_EVENT_ANY_ID,
                                           sc_event_handler,
                                           config->event_handler_arg));

    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ERROR_CHECK(esp_wifi_start());
}

void stop_smartconfig() {
    log_i(TAG_smartconfig, "stop, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif != NULL ? STATUS_OK : STATUS_FAIL);
    esp_smartconfig_stop();
    esp_wifi_disconnect();
    esp_wifi_stop();

    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, sc_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler);

    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(wifi_common.netif);
    esp_event_loop_delete_default();
    esp_netif_deinit();
    wifi_common.netif = NULL;
}

static void
wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)event_data;
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            log_i(TAG_smartconfig, "wifi start");
            ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
            ERROR_CHECK(esp_smartconfig_start(&cfg));
            smartconfig.started_event_handler(arg);
            return;

        case WIFI_EVENT_STA_DISCONNECTED:
            log_i(TAG_smartconfig, "wifi disconnected");
            smartconfig.disconnected_event_handler(arg);
            return;
        case WIFI_EVENT_STA_CONNECTED:
            log_i(TAG_smartconfig, "wifi connected");
            return;
        case WIFI_EVENT_STA_STOP:
            log_i(TAG_smartconfig, "wifi stop");
            return;
        case WIFI_EVENT_HOME_CHANNEL_CHANGE:
            log_d(TAG_smartconfig, "home channel change");
            return;

        default:
            log_w(TAG_smartconfig,
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
    log_d(TAG_smartconfig, "got ip:%s", ip4addr_ntoa((const ip4_addr_t *)&event->ip_info.ip));
    smartconfig.connected_event_handler(arg);
}

static void
sc_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)event_data;
    switch (event_id) {
        case SC_EVENT_SCAN_DONE:
            log_i(TAG_smartconfig, "Scan done");
            smartconfig.scan_done_event_handler(arg);
            return;

        case SC_EVENT_FOUND_CHANNEL:
            log_i(TAG_smartconfig, "Found channel");
            smartconfig.found_event_handler(arg);
            return;

        case SC_EVENT_GOT_SSID_PSWD: {
            smartconfig_event_got_ssid_pswd_t *evt =
                (smartconfig_event_got_ssid_pswd_t *)event_data;
            log_i(TAG_smartconfig,
                  "Got SSID and password, ssid:%s, pwd:%s",
                  evt->ssid,
                  evt->password);
            wifi_config_t wifi_config = {};

            memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
            wifi_config.sta.bssid_set = evt->bssid_set;

            if (wifi_config.sta.bssid_set == true) {
                memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
            }

            uint8_t rvd_data[65] = {};
            if (evt->type == SC_TYPE_ESPTOUCH_V2) {
                ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
                log_i(TAG_smartconfig, "RVD_DATA:%s", rvd_data);
            }

            ERROR_CHECK(esp_wifi_disconnect());
            ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_config));
            ERROR_CHECK(esp_wifi_connect());
            smartconfig.got_creds_event_handler(arg, evt->ssid, evt->password, (char *)rvd_data);
            return;
        }

        case SC_EVENT_SEND_ACK_DONE:
            log_i(TAG_smartconfig, "---- >done< ----");
            smartconfig.done_event_handler(arg);
            return;

        default:
            log_w(TAG_smartconfig,
                  "unhandled event, event_base:%s, event_id:%d",
                  event_base,
                  (int)event_id);
            break;
    }
}