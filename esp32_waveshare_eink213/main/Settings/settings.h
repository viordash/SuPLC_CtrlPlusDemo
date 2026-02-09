#pragma once

#include "DataMigrations/MigrateSettings.h"
#include <stdint.h>
#include <unistd.h>

#define INFINITY_CONNECT_RETRY -1

extern CurrentSettings::device_settings settings;

void load_settings();
void store_settings();
void delete_settings();
bool validate_settings(CurrentSettings::device_settings *settings);

void lock_settings();
void unlock_settings();

#define SAFETY_SETTINGS(action)                                                                    \
    {                                                                                              \
        lock_settings();                                                                           \
        action unlock_settings();                                                                  \
    }
