#pragma once

#include "DeviceIO.h"
#include <stdint.h>
#include <unistd.h>

typedef struct { //
    MapIO *inputs_outputs;
    size_t count;
} AllowedIO;

static inline bool ValidateMapIO(MapIO mapIO) {
    if (mapIO <= MapIO::min_bound) {
        return false;
    }
    if (mapIO >= MapIO::max_bound) {
        return false;
    }
    return true;
}

static inline int FindAllowedIO(AllowedIO *allowed, MapIO io) {
    for (size_t i = 0; i < allowed->count; i++) {
        if (allowed->inputs_outputs[i] == io) {
            return i;
        }
    }
    return -1;
}