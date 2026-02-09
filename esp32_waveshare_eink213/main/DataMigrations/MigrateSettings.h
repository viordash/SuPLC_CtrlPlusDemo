#pragma once

#include "Settings_Initial.h"

const TDataMigrate Migrate_Settings[] = {
    { INITIAL_VERSION, 0, 0, 0 },
    MigrateSettings::Initial::DataMigrate, //
};

const TDataMigrateItems SettingsMigrations = {
    Migrate_Settings,                                      //
    sizeof(Migrate_Settings) / sizeof(Migrate_Settings[0]) //
};

namespace CurrentSettings = MigrateSettings::Initial::Snapshot;