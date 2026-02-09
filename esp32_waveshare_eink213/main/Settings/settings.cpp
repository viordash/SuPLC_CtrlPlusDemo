
#include "Settings/settings.h"
#include "MigrateAnyData/MigrateAnyData.h"
#include "Storage/redundant_storage.h"
#include "os.h"
#include "partitions.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_settings = "settings";

CurrentSettings::device_settings settings = {};

static mutex_handle_t mutex = NULL;

void load_settings() {
    ERROR_CHECK(mutex == NULL ? STATUS_OK : ERR_NO_MEM);
    mutex = create_mutex();

    uint8_t *storedData = NULL;
    size_t storedSize = 0;
    uint32_t version = INITIAL_VERSION;
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       settings_storage_name);
    if (storage.size > 0) {
        version = storage.version;
        storedData = storage.data;
        storedSize = storage.size;
    }

    TMigrateResult migrateResult = MigrateData::Run(version,
                                                    DEVICE_SETTINGS_VERSION,
                                                    &SettingsMigrations,
                                                    storedData,
                                                    storedSize,
                                                    &settings,
                                                    [](void *parent, void *pItem) -> bool {
                                                        memcpy(parent, pItem, sizeof(settings));
                                                        return true;
                                                    });

    if (migrateResult == MigrateRes_Migrate) {
        log_i(TAG_settings, "migrated");
    } else if (migrateResult == MigrateRes_Skipped && storedData != NULL) {
        memcpy(&settings, storedData, sizeof(settings));
        log_i(TAG_settings, "loaded");
    } else {
        log_e(TAG_settings, "migrate error");
    }

    log_i(TAG_settings, "smartconfig.counter:%u", (unsigned int)settings.smartconfig.counter);
    log_i(TAG_settings,
          "wifi.ssid:%.*s",
          (int)sizeof(settings.wifi_station.ssid) - 1,
          settings.wifi_station.ssid);
    log_i(TAG_settings,
          "wifi.password:%.*s",
          (int)sizeof(settings.wifi_station.password) - 1,
          settings.wifi_station.password);

    delete[] storage.data;
}

void store_settings() {
    redundant_storage storage;
    storage.data = (uint8_t *)&settings;
    storage.size = sizeof(settings);
    storage.version = DEVICE_SETTINGS_VERSION;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
}

void delete_settings() {
    redundant_storage_delete(storage_0_partition,
                             storage_0_path,
                             storage_1_partition,
                             storage_1_path,
                             settings_storage_name);

    log_i(TAG_settings, "deleted");
}

static bool invalid_symbol(char ch) {
    return ch < '!' || ch > '~';
}

static bool invalid_int(int32_t value, int32_t min, int32_t max) {
    return value < min || value > max;
}

static bool invalid_uint(uint32_t value, uint32_t min, uint32_t max) {
    return value < min || value > max;
}

bool validate_settings(CurrentSettings::device_settings *settings) {
    size_t pos;
    char ch;

    pos = 0;
    while (pos < sizeof(settings->wifi_station.ssid)
           && (ch = settings->wifi_station.ssid[pos]) != 0) {
        if (invalid_symbol(ch)) {
            log_i(TAG_settings, "Invalid wifi_station.ssid, '%s'", settings->wifi_station.ssid);
            return false;
        }
        pos++;
    }

    pos = 0;
    while (pos < sizeof(settings->wifi_station.password)
           && (ch = settings->wifi_station.password[pos]) != 0) {
        if (invalid_symbol(ch)) {
            log_i(TAG_settings,
                  "Invalid wifi_station.password, '%s'",
                  settings->wifi_station.password);
            return false;
        }
        pos++;
    }
    if (invalid_int(settings->wifi_station.connect_max_retry_count, -1, 7777)) {
        log_i(TAG_settings,
              "Invalid wifi_station.connect_max_retry_count, '%d'",
              (int)settings->wifi_station.connect_max_retry_count);
        return false;
    }
    if (invalid_uint(settings->wifi_station.reconnect_delay_ms, 100, 10 * 60 * 1000)) {
        log_i(TAG_settings,
              "Invalid wifi_station.reconnect_delay_ms, '%u'",
              (unsigned int)settings->wifi_station.reconnect_delay_ms);
        return false;
    }
    if (invalid_uint(settings->wifi_station.scan_station_rssi_period_ms, 100, 10 * 60 * 1000)) {
        log_i(TAG_settings,
              "Invalid wifi_station.scan_station_rssi_period_ms, '%u'",
              (unsigned int)settings->wifi_station.scan_station_rssi_period_ms);
        return false;
    }
    if (invalid_int(settings->wifi_station.max_rssi, -120, 100)) {
        log_i(TAG_settings,
              "Invalid wifi_station.max_rssi, '%d'",
              (int)settings->wifi_station.max_rssi);
        return false;
    }
    if (invalid_int(settings->wifi_station.min_rssi, -120, 100)) {
        log_i(TAG_settings,
              "Invalid wifi_station.min_rssi, '%d'",
              (int)settings->wifi_station.min_rssi);
        return false;
    }
    if (settings->wifi_station.min_rssi > settings->wifi_station.max_rssi) {
        log_i(TAG_settings,
              "Invalid wifi_station.min_rssi > settings->wifi_station.max_rssi, '%d'>'%d'",
              (int)settings->wifi_station.min_rssi,
              (int)settings->wifi_station.max_rssi);
        return false;
    }

    if (invalid_uint(settings->wifi_scanner.per_channel_scan_time_ms, 100, 20 * 1000)) {
        log_i(TAG_settings,
              "Invalid wifi_scanner.per_channel_scan_time_ms, '%u'",
              (unsigned int)settings->wifi_scanner.per_channel_scan_time_ms);
        return false;
    }
    if (invalid_int(settings->wifi_scanner.max_rssi, -120, 100)) {
        log_i(TAG_settings,
              "Invalid wifi_scanner.max_rssi, '%d'",
              (int)settings->wifi_scanner.max_rssi);
        return false;
    }
    if (invalid_int(settings->wifi_scanner.min_rssi, -120, 100)) {
        log_i(TAG_settings,
              "Invalid wifi_scanner.min_rssi, '%d'",
              (int)settings->wifi_scanner.min_rssi);
        return false;
    }
    if (settings->wifi_scanner.min_rssi > settings->wifi_scanner.max_rssi) {
        log_i(TAG_settings,
              "Invalid wifi_scanner.min_rssi > settings->wifi_scanner.max_rssi, '%d'>'%d'",
              (int)settings->wifi_scanner.min_rssi,
              (int)settings->wifi_scanner.max_rssi);
        return false;
    }

    if (invalid_uint(settings->wifi_access_point.generation_time_ms, 100, 10 * 60 * 1000)) {
        log_i(TAG_settings,
              "Invalid wifi_access_point.generation_time_ms, '%u'",
              (unsigned int)settings->wifi_access_point.generation_time_ms);
        return false;
    }
    if (invalid_uint(settings->wifi_access_point.ssid_hidden, 0, 1)) {
        log_i(TAG_settings,
              "Invalid wifi_access_point.ssid_hidden, '%u'",
              (unsigned int)settings->wifi_access_point.ssid_hidden);
        return false;
    }
    if (invalid_uint(settings->adc.scan_period_ms, 20, 60 * 1000)) {
        log_i(TAG_settings,
              "Invalid adc.scan_period_ms, '%u'",
              (unsigned int)settings->adc.scan_period_ms);
        return false;
    }
    return true;
}

void lock_settings() {
    ERROR_CHECK(!lock_mutex(mutex) ? ERR_NO_MEM : STATUS_OK);
}

void unlock_settings() {
    unlock_mutex(mutex);
}