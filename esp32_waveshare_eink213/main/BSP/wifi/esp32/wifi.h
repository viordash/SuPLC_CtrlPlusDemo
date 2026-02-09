#pragma once

#include "os.h"
#include <stdint.h>
#include <unistd.h>

#define MAC_BYTES 6

typedef void (*wifi_sta_connect_event_handler_t)(void *arg);
typedef void (*wifi_sta_fail_event_handler_t)(void *arg);

typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
    wifi_sta_connect_event_handler_t connect_event_handler;
    wifi_sta_fail_event_handler_t fail_event_handler;
    void *event_handler_arg;
} wifi_station_config_t;

void wifi_start_station(wifi_station_config_t *config);
void wifi_stop_station();
bool get_wifi_station_rssi(int8_t *rssi);
void get_wifi_station_creds(char *ssid, size_t ssid_size, char *password, size_t password_size);

typedef struct {
    uint8_t bssid[6];
    uint8_t ssid[33];
    uint8_t primary;
    int8_t rssi;
} wifi_scan_record_t;

void wifi_start_scan(const char *ssid, uint16_t per_channel_scan_time_ms);
void wifi_stop_scan();
error_t wifi_scan_get_records(uint16_t *number, wifi_scan_record_t *ap_records);

typedef bool (*wifi_hotspot_connect_event_handler_t)(void *arg,
                                                     uint8_t aid,
                                                     const uint8_t mac[MAC_BYTES]);
typedef void (*wifi_hotspot_disconnect_event_handler_t)(void *arg,
                                                        uint8_t aid,
                                                        const uint8_t mac[MAC_BYTES]);

typedef struct {
    const char *ssid;
    const char *password;
    bool secure_client;
    bool ssid_hidden;
    uint8_t max_connection;
    wifi_hotspot_connect_event_handler_t connect_event_handler;
    wifi_hotspot_disconnect_event_handler_t disconnect_event_handler;
    void *event_handler_arg;
} wifi_hotspot_config_t;

void wifi_start_hotspot(wifi_hotspot_config_t *config);
void wifi_stop_hotspot();
