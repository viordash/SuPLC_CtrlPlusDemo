

#include "board.h"
#include "os.h"

static const char *TAG_wifi_hotspot = "wifi_hotspot";

static struct {
    wifi_hotspot_connect_event_handler_t connect_event_handler;
    wifi_hotspot_disconnect_event_handler_t disconnect_event_handler;
} wifi_hotspot;

static void hotspot_connect_event_handler(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id,
                                          void *event_data);
static void hotspot_disconnect_event_handler(void *arg,
                                             esp_event_base_t event_base,
                                             int32_t event_id,
                                             void *event_data);

void wifi_start_hotspot(wifi_hotspot_config_t *config) {
    log_i(TAG_wifi_hotspot, "start, %p, ssid:'%s'", wifi_common.netif, config->ssid);
    ERROR_CHECK(wifi_common.netif == NULL ? STATUS_OK : STATUS_FAIL);

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.ap.ssid, config->ssid);
    wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);
    if (config->secure_client) {
        strcpy((char *)wifi_config.ap.password, config->password);
    }
    wifi_config.ap.authmode = config->secure_client ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    wifi_config.ap.max_connection = config->max_connection;
    wifi_config.ap.ssid_hidden = config->ssid_hidden;

    wifi_hotspot.connect_event_handler = config->connect_event_handler;
    wifi_hotspot.disconnect_event_handler = config->disconnect_event_handler;

    ERROR_CHECK(esp_netif_init());
    ERROR_CHECK(esp_event_loop_create_default());

    wifi_common.netif = esp_netif_create_default_wifi_sta();
    assert(wifi_common.netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ERROR_CHECK(esp_wifi_init(&cfg));
    ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                           WIFI_EVENT_AP_STACONNECTED,
                                           &hotspot_connect_event_handler,
                                           &config->event_handler_arg));
    ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                           WIFI_EVENT_AP_STADISCONNECTED,
                                           &hotspot_disconnect_event_handler,
                                           &config->event_handler_arg));

    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ERROR_CHECK(esp_wifi_start());
}

void wifi_stop_hotspot() {
    log_i(TAG_wifi_hotspot, "stop, %p", wifi_common.netif);
    ERROR_CHECK(wifi_common.netif != NULL ? STATUS_OK : STATUS_FAIL);
    esp_wifi_disconnect();
    esp_wifi_stop();

    esp_event_handler_unregister(WIFI_EVENT,
                                 WIFI_EVENT_AP_STACONNECTED,
                                 &hotspot_connect_event_handler);
    esp_event_handler_unregister(WIFI_EVENT,
                                 WIFI_EVENT_AP_STADISCONNECTED,
                                 &hotspot_disconnect_event_handler);

    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(wifi_common.netif);
    esp_event_loop_delete_default();
    esp_netif_deinit();
    wifi_common.netif = NULL;
}

static void hotspot_connect_event_handler(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id,
                                          void *event_data) {
    (void)event_base;
    (void)event_id;
    auto event = (wifi_event_ap_staconnected_t *)event_data;

    if (!wifi_hotspot.connect_event_handler(arg, event->aid, event->mac)) {
        esp_wifi_deauth_sta(event->aid);
    }
}

static void hotspot_disconnect_event_handler(void *arg,
                                             esp_event_base_t event_base,
                                             int32_t event_id,
                                             void *event_data) {
    (void)event_base;
    (void)event_id;
    auto event = (wifi_event_ap_stadisconnected_t *)event_data;

    wifi_hotspot.disconnect_event_handler(arg, event->aid, event->mac);
}
