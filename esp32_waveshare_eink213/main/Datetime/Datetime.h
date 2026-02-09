#pragma once

#include <stdint.h>
#include <unistd.h>

typedef struct {
    int32_t second;
    int32_t minute;
    int32_t hour;
    int32_t day;
    int32_t month;
    int32_t year;
} Datetime;

static inline bool ValidateDatetime(Datetime *dt) {
    if (dt->year > 2100 || dt->year < 2020) {
        return false;
    }
    if (dt->month > 12 || dt->month < 1) {
        return false;
    }
    if (dt->day > 31 || dt->day < 1) {
        return false;
    }

    if (dt->hour >= 24 || dt->hour < 0) {
        return false;
    }
    if (dt->minute >= 60 || dt->minute < 0) {
        return false;
    }
    if (dt->second >= 60 || dt->second < 0) {
        return false;
    }
    return true;
}