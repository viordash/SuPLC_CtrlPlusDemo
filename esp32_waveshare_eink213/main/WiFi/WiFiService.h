#pragma once

#include "Settings/settings.h"
#include "WiFiRequests.h"
#include "board.h"
#include "os.h"
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

using WifiStationOnConnect = std::function<void(void)>;
using WifiStationOnDisConnect = std::function<void(void)>;

class WiFiService {
  public:
    struct AccessPointEventArg {
        WiFiService *service;
        const char *ssid;
        const char *mac;
    };

    using t_mac = uint64_t;

  protected:
    WifiStationOnConnect on_connect_station;
    WifiStationOnDisConnect on_disconnect_station;
    event_t events;
    WiFiRequests requests;

    std::mutex scanned_ssid_lock_mutex;
    std::unordered_map<const char *, uint8_t> scanned_ssid;

    uint8_t station_rssi;

    std::mutex ap_clients_lock_mutex;
    std::unordered_map<const char *, std::unordered_set<t_mac>> ap_clients;

    std::mutex station_connect_status_lock_mutex;

    void StationTask(RequestItem *request);
    bool ObtainStationRssi();
    static void sta_connect_event_handler(void *arg);
    static void sta_fail_event_handler(void *arg);

    int8_t Scanning(RequestItem *request,
                    CurrentSettings::wifi_scanner_settings *scanner_settings,
                    bool *canceled);
    void ScannerTask(RequestItem *request);

    void AccessPointTask(RequestItem *request);
    static bool ap_connect_event_handler(void *arg, uint8_t aid, const uint8_t mac[MAC_BYTES]);
    static void ap_disconnect_event_handler(void *arg, uint8_t aid, const uint8_t mac[MAC_BYTES]);

    static void Task(void *parm);

    uint8_t ScaleRssiToPercent04(int8_t rssi, int8_t max_rssi, int8_t min_rssi);
    void AddScannedSsid(const char *ssid, uint8_t rssi);
    bool FindScannedSsid(const char *ssid, uint8_t *rssi);
    void RemoveScannedSsid(const char *ssid);

    void AddApClient(const char *ssid, t_mac mac);
    size_t GetApClientsCount(const char *ssid);
    void RemoveApClient(const char *ssid, t_mac mac);
    void RemoveApClients(const char *ssid);

  public:
    static const int STOP_BIT = BIT0;
    static const int FAILED_BIT = BIT1;
    static const int CONNECTED_BIT = BIT2;
    static const int CANCEL_REQUEST_BIT = BIT3;

    WiFiService(WifiStationOnConnect on_connect_station,
                WifiStationOnDisConnect on_disconnect_station);
    ~WiFiService();

    void Start();

    uint8_t ConnectToStation();
    void DisconnectFromStation();

    uint8_t Scan(const char *ssid);
    void CancelScan(const char *ssid);

    size_t AccessPoint(const char *ssid, const char *password, const char *mac);
    void CancelAccessPoint(const char *ssid);
};
