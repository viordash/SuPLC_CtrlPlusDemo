#include "WiFiService.h"
#include "LogicProgram/LogicElement.h"
#include "Settings/settings.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService = "WiFiService";
extern CurrentSettings::device_settings settings;

WiFiService::WiFiService(WifiStationOnConnect on_connect_station,
                         WifiStationOnDisConnect on_disconnect_station)
    : on_connect_station{on_connect_station},on_disconnect_station{on_disconnect_station},  events{ create_event() }, station_rssi{ LogicElement::MinValue } {
}

WiFiService::~WiFiService() {
    delete_event(events);
}

void WiFiService::Start() {
    task_handle_t task_handle =
        create_task(Task, "wifi_task", WiFiService_Task_Stack_size, this, task_priority_t::tp_Low);

    subscribe_to_event(events, task_handle);
    log_w(TAG_WiFiService, "Start");
}

uint8_t WiFiService::ConnectToStation() {
    if (requests.Station()) {
        raise_wakeup_event(events);
        log_d(TAG_WiFiService, "ConnectToStation");
    }
    log_d(TAG_WiFiService, "ConnectToStation, rssi:%u", station_rssi);
    return station_rssi;
}

void WiFiService::DisconnectFromStation() {
    bool removed = requests.RemoveStation();
    log_i(TAG_WiFiService, "DisconnectFromStation, removed:%d", removed);
    if (removed) {
        raise_event(events, CANCEL_REQUEST_BIT);
    }
}

uint8_t WiFiService::Scan(const char *ssid) {
    uint8_t rssi;
    bool found = FindScannedSsid(ssid, &rssi);
    if (!found) {
        rssi = LogicElement::MinValue;
    }

    if (requests.Scan(ssid)) {
        raise_wakeup_event(events);
        log_d(TAG_WiFiService,
              "Scan, ssid:%s, found:%u, rssi:%u",
              ssid,
              (unsigned int)found,
              (unsigned int)rssi);
    }
    return rssi;
}

void WiFiService::CancelScan(const char *ssid) {
    RemoveScannedSsid(ssid);
    bool removed = requests.RemoveScanner(ssid);
    log_i(TAG_WiFiService, "CancelScan, ssid:%s, removed:%u", ssid, (unsigned int)removed);
    if (removed) {
        raise_event(events, CANCEL_REQUEST_BIT);
    }
}

size_t WiFiService::AccessPoint(const char *ssid, const char *password, const char *mac) {
    if (requests.AccessPoint(ssid, password, mac)) {
        raise_wakeup_event(events);
        log_d(TAG_WiFiService, "AccessPoint, ssid:%s", ssid);
    }
    return GetApClientsCount(ssid);
}

void WiFiService::CancelAccessPoint(const char *ssid) {
    RemoveApClients(ssid);
    bool removed = requests.RemoveAccessPoint(ssid);
    log_i(TAG_WiFiService, "CancelAccessPoint, ssid:%s, removed:%u", ssid, (unsigned int)removed);
    if (removed) {
        raise_event(events, CANCEL_REQUEST_BIT);
    }
}

void WiFiService::Task(void *parm) {
    log_i(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    uint32_t ulNotifiedValue = 0;
    while (true) {
        wait_event(wifi_service->events, &ulNotifiedValue);
        if ((ulNotifiedValue & STOP_BIT) != 0) {
            break;
        }
        log_d(TAG_WiFiService, "new request, uxBits:0x%08X", (unsigned int)ulNotifiedValue);
        RequestItem new_request;
        while (wifi_service->requests.Pop(&new_request)) {
            log_i(TAG_WiFiService, "exec request, type:%u", (unsigned int)new_request.Type);

            switch (new_request.Type) {
                case wqi_Station:
                    wifi_service->StationTask(&new_request);
                    break;

                case wqi_Scanner:
                    wifi_service->ScannerTask(&new_request);
                    break;

                case wqi_AccessPoint:
                    wifi_service->AccessPointTask(&new_request);
                    break;

                default:
                    break;
            }
            log_d(TAG_WiFiService, "end request, type:%u", (unsigned int)new_request.Type);
        }
    }

    log_w(TAG_WiFiService, "Finish task");
    delete_current_task();
}
