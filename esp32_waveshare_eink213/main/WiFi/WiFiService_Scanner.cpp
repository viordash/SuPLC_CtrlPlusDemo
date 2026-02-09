

#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "WiFiService.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_Scanner = "WiFiService.Scanner";
extern CurrentSettings::device_settings settings;

int8_t WiFiService::Scanning(RequestItem *request,
                             CurrentSettings::wifi_scanner_settings *scanner_settings,
                             bool *canceled) {
    error_t err;

    wifi_scan_record_t ap_info[1] = {};

    uint32_t scan_max_time_ms = (scanner_settings->per_channel_scan_time_ms * WiFi_Channels_Count);
    auto current_time = timer_get_time_us();
    auto timeout_time = current_time + (scan_max_time_ms * 1000);

    uint32_t ulNotifiedValue = 0;
    while (true) {
        uint16_t number = sizeof(ap_info) / sizeof(ap_info[0]);
        err = wifi_scan_get_records(&number, ap_info);
        if (err != STATUS_OK) {
            log_e(TAG_WiFiService_Scanner, "wifi_scan_get_records err 0x%X", (unsigned int)err);
            break;
        }

        for (int i = 0; i < number; i++) {
            log_i(TAG_WiFiService_Scanner,
                  "SSID:'%s', RSSI:%d, Channel:%d, reqSSID:'%s'",
                  ap_info[i].ssid,
                  ap_info[i].rssi,
                  ap_info[i].primary,
                  request->Payload.Scanner.ssid);
            if (strcmp((const char *)ap_info[i].ssid, (const char *)request->Payload.Scanner.ssid)
                    == 0
                && ap_info[i].rssi > scanner_settings->min_rssi) {
                return ap_info[i].rssi;
            }
        }

        bool notify_wait_timeout =
            !wait_event_timed(events, &ulNotifiedValue, scanner_settings->per_channel_scan_time_ms);

        int64_t timespan = timeout_time - timer_get_time_us();
        log_d(TAG_WiFiService_Scanner,
              "process, uxBits:0x%08X, timespan:%lld",
              (unsigned int)ulNotifiedValue,
              (long long)timespan);

        if (timespan <= 0) {
            break;
        }
        bool to_stop = !notify_wait_timeout && (ulNotifiedValue & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        *canceled = !notify_wait_timeout && (ulNotifiedValue & CANCEL_REQUEST_BIT) != 0
                 && !requests.Contains(request);
        if (*canceled) {
            log_i(TAG_WiFiService_Scanner,
                  "Cancel request, ssid:%s",
                  request->Payload.Scanner.ssid);
            break;
        }
    }
    return scanner_settings->min_rssi;
}

void WiFiService::ScannerTask(RequestItem *request) {
    log_i(TAG_WiFiService_Scanner, "start, ssid:%s", request->Payload.Scanner.ssid);

    CurrentSettings::wifi_scanner_settings scanner_settings;
    SAFETY_SETTINGS({ scanner_settings = settings.wifi_scanner; });

    bool canceled = false;
    wifi_start_scan(request->Payload.Scanner.ssid, scanner_settings.per_channel_scan_time_ms);
    int8_t rssi = Scanning(request, &scanner_settings, &canceled);
    wifi_stop_scan();

    if (rssi > scanner_settings.min_rssi) {
        AddScannedSsid(
            request->Payload.Scanner.ssid,
            ScaleRssiToPercent04(rssi, scanner_settings.max_rssi, scanner_settings.min_rssi));
    } else {
        RemoveScannedSsid(request->Payload.Scanner.ssid);
    }
    requests.RemoveScanner(request->Payload.Scanner.ssid);
    if (!canceled) {
        Controller::WakeupProcessTask();
    }

    log_i(TAG_WiFiService_Scanner,
          "finish, ssid:%s, rssi:%d[%d]",
          request->Payload.Scanner.ssid,
          (int)rssi,
          ScaleRssiToPercent04(rssi, scanner_settings.max_rssi, scanner_settings.min_rssi));
}

uint8_t WiFiService::ScaleRssiToPercent04(int8_t rssi, int8_t max_rssi, int8_t min_rssi) {
    float fl = ((float)rssi - min_rssi) / (max_rssi - min_rssi);
    fl = fl * (LogicElement::MaxValue - LogicElement::MinValue) + LogicElement::MinValue;
    if (fl < (float)LogicElement::MinValue) {
        fl = (float)LogicElement::MinValue;
    } else if (fl > (float)LogicElement::MaxValue) {
        fl = (float)LogicElement::MaxValue;
    }
    return (uint8_t)fl;
}

void WiFiService::AddScannedSsid(const char *ssid, uint8_t rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.insert({ ssid, rssi });
    if (!it.second) {
        it.first->second = rssi;
    }
    log_d(TAG_WiFiService_Scanner, "AddScannedSsid, cnt:%u", (unsigned int)scanned_ssid.size());
}

bool WiFiService::FindScannedSsid(const char *ssid, uint8_t *rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.find(ssid);
    bool found = it != scanned_ssid.end();
    log_d(TAG_WiFiService_Scanner, "FindScannedSsid, found:%u", (unsigned int)found);
    if (found) {
        *rssi = it->second;
    }
    return found;
}

void WiFiService::RemoveScannedSsid(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.erase(ssid);
    log_d(TAG_WiFiService_Scanner, "RemoveScannedSsid, cnt:%u", (unsigned int)scanned_ssid.size());
}