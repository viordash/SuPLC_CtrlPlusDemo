#pragma once

#include "Datetime/Datetime.h"
#include <stdint.h>
#include <unistd.h>

typedef struct {
    int32_t is_hotstart;
    uint32_t restart_count;
    int32_t view_top_index;
    int32_t selected_network;
    Datetime current_datetime;
} hotreload_data;

extern hotreload_data *hotreload;

void load_hotreload();
void store_hotreload();

void lock_hotreload();
void unlock_hotreload();

#define SAFETY_HOTRELOAD(action)                                                                   \
    {                                                                                              \
        lock_hotreload();                                                                          \
        action unlock_hotreload();                                                                 \
    }
