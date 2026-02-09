#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "os.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace MigrateSettings {
    namespace Initial {
        inline int GetSizeOfCurrentData();
        inline void MigrateUp(void *pCurr, void *pPrev);
        inline void MigrateDown(void *pCurr, void *pPrev);

        const TDataMigrate DataMigrate = { 0x00000001,
                                           MigrateUp,
                                           MigrateDown,
                                           GetSizeOfCurrentData };

        namespace Snapshot {
            typedef struct {
                uint32_t counter;
            } smartconfig_settings;

            typedef struct {
                char ssid[32];
                char password[64];
                int32_t connect_max_retry_count;
                uint32_t reconnect_delay_ms;
                uint32_t scan_station_rssi_period_ms;
                int8_t max_rssi;
                int8_t min_rssi;
                uint32_t min_worktime_ms;
            } wifi_station_settings;

            typedef struct {
                uint16_t per_channel_scan_time_ms;
                int8_t max_rssi;
                int8_t min_rssi;
            } wifi_scanner_settings;

            typedef struct {
                uint32_t generation_time_ms;
                bool ssid_hidden;
            } wifi_access_point_settings;

            typedef struct {
                char sntp_server_primary[64];
                char sntp_server_secondary[64];
                char timezone[32];
            } datetime_settings;

            typedef struct {
                uint32_t scan_period_ms;
            } adc_settings;

            typedef struct {
                smartconfig_settings smartconfig;
                wifi_station_settings wifi_station;
                wifi_scanner_settings wifi_scanner;
                wifi_access_point_settings wifi_access_point;
                datetime_settings datetime;
                adc_settings adc;
            } device_settings;
        } // namespace Snapshot

        inline int GetSizeOfCurrentData() {
            return sizeof(Snapshot::device_settings);
        }

        inline void MigrateUp(void *pCurr, void *pPrev) {
            (void)pPrev;
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;

            pCurrSettings->smartconfig.counter = 0;

            memset(pCurrSettings->wifi_station.ssid, 0, sizeof(pCurrSettings->wifi_station.ssid));
            memset(pCurrSettings->wifi_station.password,
                   0,
                   sizeof(pCurrSettings->wifi_station.password));
            pCurrSettings->wifi_station.connect_max_retry_count = -1;
            pCurrSettings->wifi_station.reconnect_delay_ms = 3000;
            pCurrSettings->wifi_station.scan_station_rssi_period_ms = 5000;
            pCurrSettings->wifi_station.max_rssi = -26;
            pCurrSettings->wifi_station.min_rssi = -120;
            pCurrSettings->wifi_station.min_worktime_ms = 10000;

            pCurrSettings->wifi_scanner.per_channel_scan_time_ms = 500;
            pCurrSettings->wifi_scanner.max_rssi = -26;
            pCurrSettings->wifi_scanner.min_rssi = -120;

            pCurrSettings->wifi_access_point.generation_time_ms = 20000;
            pCurrSettings->wifi_access_point.ssid_hidden = false;

            strcpy(pCurrSettings->datetime.sntp_server_primary, "ru.pool.ntp.org");
            strcpy(pCurrSettings->datetime.sntp_server_secondary, "pool.ntp.org");
            strcpy(pCurrSettings->datetime.timezone, "GMT-3");

            pCurrSettings->adc.scan_period_ms = 1000;

            log_i("Settings_Initial",
                  "Settings. Initial migrate to %04X",
                  (unsigned int)DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            (void)pCurr;
            (void)pPrev;
        }

    } // namespace Initial
} // namespace MigrateSettings
