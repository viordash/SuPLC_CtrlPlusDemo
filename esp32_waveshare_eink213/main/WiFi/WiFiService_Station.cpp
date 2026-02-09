
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "Settings/settings.h"
#include "WiFiService.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_Station = "WiFiService.Station";
extern CurrentSettings::device_settings settings;

void WiFiService::StationTask(RequestItem *request) {
    log_d(TAG_WiFiService_Station, "start");

    int connect_retries_num = 0;
    int32_t max_retry_count;
    uint32_t reconnect_delay_ms;
    uint32_t scan_station_rssi_period_ms;
    uint32_t min_worktime_ms;
    wifi_station_config_t config = {};
    SAFETY_SETTINGS({
        memcpy(config.ssid, settings.wifi_station.ssid, sizeof(config.ssid)); //
        memcpy(config.password, settings.wifi_station.password,
               sizeof(config.password)); //
        max_retry_count = settings.wifi_station.connect_max_retry_count;
        reconnect_delay_ms = settings.wifi_station.reconnect_delay_ms;
        scan_station_rssi_period_ms = settings.wifi_station.scan_station_rssi_period_ms;
        min_worktime_ms = settings.wifi_station.min_worktime_ms;
    });

    bool has_wifi_sta_settings = config.ssid[0] != 0;
    if (!has_wifi_sta_settings) {
        log_w(TAG_WiFiService_Station, "no creds saved");
        requests.RemoveStation();
        station_rssi = LogicElement::MinValue;
        return;
    }
    config.connect_event_handler = &sta_connect_event_handler;
    config.fail_event_handler = &sta_fail_event_handler;
    config.event_handler_arg = this;
    wifi_start_station(&config);

    bool has_connect = false;
    uint64_t connection_start_time = 0;
    uint32_t ulNotifiedValue = 0;

    bool cancel = false;
    while (true) {
        if (ulNotifiedValue == 0) {
            if (!wait_event_timed(events, &ulNotifiedValue, scan_station_rssi_period_ms)) {
                if (has_connect && !ObtainStationRssi()) {
                    ulNotifiedValue = FAILED_BIT;
                }
            }
        }
        log_d(TAG_WiFiService_Station, "event uxBits:0x%08X", (unsigned int)ulNotifiedValue);
        uint32_t notified_event = ulNotifiedValue;
        ulNotifiedValue = 0;

        bool to_stop = (notified_event & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        cancel = (notified_event & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            log_i(TAG_WiFiService_Station, "Cancel");
            break;
        }

        bool one_more_request = requests.OneMoreInQueue();
        bool any_failure = (notified_event & FAILED_BIT) != 0;
        if (any_failure) {

            has_connect = false;
            bool retry_connect = (max_retry_count == INFINITY_CONNECT_RETRY
                                  || connect_retries_num < max_retry_count);
            if (!retry_connect) {
                log_w(TAG_WiFiService_Station, "failed. unable reconnect");
                station_rssi = LogicElement::MinValue;
                on_disconnect_station();
                wifi_stop_station();
                Controller::WakeupProcessTask();
                wait_event(events, &ulNotifiedValue);
            } else {
                const int retries_num_before_no_station = 3;
                if (connect_retries_num >= retries_num_before_no_station) {
                    station_rssi = LogicElement::MinValue;
                    Controller::WakeupProcessTask();
                    if (one_more_request) {
                        log_i(TAG_WiFiService_Station,
                              "Stop connecting to station due to new request");
                        break;
                    }
                }

                connect_retries_num++;
                log_i(TAG_WiFiService_Station,
                      "'%s' failed. reconnect, num:%d of %d",
                      settings.wifi_station.ssid,
                      connect_retries_num,
                      (int)max_retry_count);

                on_disconnect_station();
                wifi_stop_station();
                bool delay_before_reconnect =
                    !wait_event_timed(events, &ulNotifiedValue, reconnect_delay_ms);
                if (delay_before_reconnect) {
                    wifi_start_station(&config);
                    ulNotifiedValue = 0;
                }
            }
            continue;
        }

        bool connected = (notified_event & CONNECTED_BIT) != 0;
        if (connected) {
            on_connect_station();
            connect_retries_num = 0;
            has_connect = true;
            connection_start_time = timer_get_time_us();
            if (ObtainStationRssi()) {
                Controller::WakeupProcessTask();
            }
            log_d(TAG_WiFiService_Station, "ConnectToStation, rssi:%u", station_rssi);
        }

        if (one_more_request && has_connect) {
            int64_t timespan =
                (connection_start_time + (min_worktime_ms * 1000)) - timer_get_time_us();

            if (timespan > 0) {
                const uint32_t delay_before_disconnect_ms = timespan / 1000;
                log_i(TAG_WiFiService_Station,
                      "Wait %u ms before disconnect",
                      (unsigned int)delay_before_disconnect_ms);

                wait_event_timed(events, &ulNotifiedValue, delay_before_disconnect_ms);
            }
            log_i(TAG_WiFiService_Station, "Disconnect station due to new request");
            break;
        }
    }
    on_disconnect_station();
    wifi_stop_station();

    const uint32_t wait_disconnection_ms = 500;
    if (!wait_event_timed(events, &ulNotifiedValue, wait_disconnection_ms)) {
        log_i(TAG_WiFiService_Station, "not fully disconnected");
    }

    requests.RemoveStation();
    if (!cancel) {
        requests.Station();
    }
    log_d(TAG_WiFiService_Station, "finish");
}

bool WiFiService::ObtainStationRssi() {
    int8_t rssi;
    if (!get_wifi_station_rssi(&rssi)) {
        log_i(TAG_WiFiService_Station, "ObtainStationRssi, no connection");
        return false;
    }

    CurrentSettings::wifi_station_settings wifi_station;
    SAFETY_SETTINGS({ wifi_station = settings.wifi_station; });

    station_rssi = ScaleRssiToPercent04(rssi, wifi_station.max_rssi, wifi_station.min_rssi);
    log_d(TAG_WiFiService_Station, "rssi:%d[%u]", (int)rssi, (unsigned int)station_rssi);
    return true;
}

void WiFiService::sta_connect_event_handler(void *arg) {
    auto wifi_service = static_cast<WiFiService *>(arg);
    raise_event(wifi_service->events, CONNECTED_BIT);
}
void WiFiService::sta_fail_event_handler(void *arg) {
    auto wifi_service = static_cast<WiFiService *>(arg);
    raise_event(wifi_service->events, FAILED_BIT);
}
