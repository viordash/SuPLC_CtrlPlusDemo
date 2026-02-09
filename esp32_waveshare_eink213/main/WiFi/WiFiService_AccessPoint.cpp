
#include "LogicProgram/Bindings/WiFiApBinding.h"
#include "LogicProgram/Controller.h"
#include "Settings/settings.h"
#include "WiFiService.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_AccessPoint = "WiFiService.AccessPoint";
extern CurrentSettings::device_settings settings;

void WiFiService::AccessPointTask(RequestItem *request) {
    AccessPointEventArg ap_event_arg = { this,
                                         request->Payload.AccessPoint.ssid,
                                         request->Payload.AccessPoint.mac };
    CurrentSettings::wifi_access_point_settings access_point_settings;
    SAFETY_SETTINGS({ access_point_settings = settings.wifi_access_point; });

    wifi_hotspot_config_t config = {};
    config.ssid = request->Payload.AccessPoint.ssid;
    config.password = request->Payload.AccessPoint.password;
    config.secure_client = request->Payload.AccessPoint.password != NULL
                        && strlen(request->Payload.AccessPoint.password) > 0;
    config.ssid_hidden = access_point_settings.ssid_hidden;
    config.max_connection = config.secure_client ? WiFi_Hotspot_Max_Clients : 0;
    config.connect_event_handler = &ap_connect_event_handler;
    config.disconnect_event_handler = &ap_disconnect_event_handler;
    config.event_handler_arg = &ap_event_arg;

    log_i(TAG_WiFiService_AccessPoint,
          "start, ssid:'%s', password:'%s', mac:'%s'",
          request->Payload.AccessPoint.ssid,
          config.secure_client ? request->Payload.AccessPoint.password : "",
          config.secure_client ? request->Payload.AccessPoint.mac : "");

    wifi_start_hotspot(&config);

    log_i(TAG_WiFiService_AccessPoint,
          "generating ssid:'%s'...",
          request->Payload.AccessPoint.ssid);

    bool cancel = false;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        bool notify_wait_timeout =
            !wait_event_timed(events, &ulNotifiedValue, access_point_settings.generation_time_ms);

        log_d(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", (unsigned int)ulNotifiedValue);

        if (notify_wait_timeout && requests.OneMoreInQueue()) {
            log_i(TAG_WiFiService_AccessPoint, "Stop AP due to new request");
            break;
        }

        bool to_stop = (ulNotifiedValue & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        cancel = (ulNotifiedValue & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            log_i(TAG_WiFiService_AccessPoint,
                  "Cancel request, ssid:%s",
                  request->Payload.AccessPoint.ssid);
            break;
        }
    }

    wifi_stop_hotspot();

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    if (!cancel) {
        requests.AccessPoint(request->Payload.AccessPoint.ssid,
                             request->Payload.AccessPoint.password,
                             request->Payload.AccessPoint.mac);
    }

    log_i(TAG_WiFiService_AccessPoint, "finish");
}

bool WiFiService::ap_connect_event_handler(void *arg, uint8_t aid, const uint8_t mac[MAC_BYTES]) {
    auto ap_event_arg = static_cast<AccessPointEventArg *>(arg);
    log_i(TAG_WiFiService_AccessPoint,
          "connect client aid:%u, mac:" MACSTR ", mask:%s",
          (unsigned int)aid,
          MAC2STR(mac),
          ap_event_arg->mac);

    bool matched = WiFiApBinding::ClientMacMatches(ap_event_arg->mac, mac);
    if (matched) {
        t_mac client_mac = {};
        memcpy(&client_mac, mac, MAC_BYTES);
        ap_event_arg->service->AddApClient(ap_event_arg->ssid, client_mac);
        Controller::WakeupProcessTask();
    }
    return matched;
}

void WiFiService::ap_disconnect_event_handler(void *arg,
                                              uint8_t aid,
                                              const uint8_t mac[MAC_BYTES]) {
    auto ap_event_arg = static_cast<AccessPointEventArg *>(arg);

    log_i(TAG_WiFiService_AccessPoint,
          "disconnect client aid:%u, mac:" MACSTR ", mask:%s",
          (unsigned int)aid,
          MAC2STR(mac),
          ap_event_arg->mac);

    if (WiFiApBinding::ClientMacMatches(ap_event_arg->mac, mac)) {
        t_mac client_mac = {};
        memcpy(&client_mac, mac, MAC_BYTES);
        ap_event_arg->service->RemoveApClient(ap_event_arg->ssid, client_mac);
        Controller::WakeupProcessTask();
    }
}

void WiFiService::AddApClient(const char *ssid, t_mac mac) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.insert({ ssid, { mac } });
    if (!it.second) {
        it.first->second.insert(mac);
    }
    log_i(TAG_WiFiService_AccessPoint,
          "AddApClient, ssid_cnt: %u, ssid_clients:%u",
          (unsigned int)ap_clients.size(),
          (unsigned int)it.first->second.size());
}

size_t WiFiService::GetApClientsCount(const char *ssid) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.find(ssid);
    bool found = it != ap_clients.end();
    log_d(TAG_WiFiService_AccessPoint, "FindApClient, found:%u", (unsigned int)found);
    if (found) {
        return it->second.size();
    }
    return 0;
}

void WiFiService::RemoveApClient(const char *ssid, t_mac mac) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.find(ssid);
    bool found = it != ap_clients.end();
    log_d(TAG_WiFiService_AccessPoint, "FindApClient, found:%u", (unsigned int)found);
    if (found) {
        it->second.erase(mac);
        bool empty_ssid_to_be_delete = it->second.size() == 0;
        if (empty_ssid_to_be_delete) {
            ap_clients.erase(ssid);
        }
    }
    log_i(TAG_WiFiService_AccessPoint, "RemoveApClient, cnt:%u", (unsigned int)ap_clients.size());
}

void WiFiService::RemoveApClients(const char *ssid) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    ap_clients.erase(ssid);
    log_i(TAG_WiFiService_AccessPoint, "RemoveApClients, cnt:%u", (unsigned int)ap_clients.size());
}
