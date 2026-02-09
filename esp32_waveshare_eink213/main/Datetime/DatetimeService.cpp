#include "DatetimeService.h"
#include "Hotreload/hotreload_service.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "Settings/settings.h"
#include "board.h"
#include "os.h"
#include "sys/time.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char *TAG_DatetimeService = "DatetimeService";
extern CurrentSettings::device_settings settings;

DatetimeService::DatetimeService() : events{ create_event() } {
}

DatetimeService::~DatetimeService() {
    delete_event(events);
}

void DatetimeService::Start() {
    task_handle_t task_handle = create_task(Task,
                                            "datetime_task",
                                            DatetimeService_Task_Stack_size,
                                            this,
                                            task_priority_t::tp_Idle);

    subscribe_to_event(events, task_handle);
}

void DatetimeService::Task(void *parm) {
    log_i(TAG_DatetimeService, "Start task");
    auto datetime_service = static_cast<DatetimeService *>(parm);

    const uint32_t default_update_time_ms = 60000;
    const uint32_t no_wifi_station_update_time_ms = 10000;
    Datetime datetime;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        uint32_t update_time_ms = default_update_time_ms;
        bool use_ntp = datetime_service->EnableSntp();
        if (use_ntp) {
            bool restart_ntp = (ulNotifiedValue & RESTART_SNTP_BIT) != 0;
            if (restart_ntp || !datetime_service->SntpInProcess()) {
                if (Controller::ConnectToWiFiStation() > LogicElement::MinValue) {
                    datetime_service->StopSntp();
                    datetime_service->StartSntp();
                } else {
                    log_w(TAG_DatetimeService, "no wifi station");
                    update_time_ms = no_wifi_station_update_time_ms;
                }
            }
        }

        datetime_service->Get(&datetime);
        bool datetime_are_valid = ValidateDatetime(&datetime);
        if ((ulNotifiedValue & RESTART_SNTP_BIT) == 0 || (ulNotifiedValue & STORE_BIT) != 0) {
            if (datetime_are_valid) {
                SAFETY_HOTRELOAD({
                    hotreload->current_datetime.year = datetime.year;
                    hotreload->current_datetime.month = datetime.month;
                    hotreload->current_datetime.day = datetime.day;
                    hotreload->current_datetime.hour = datetime.hour;
                    hotreload->current_datetime.minute = datetime.minute;
                    hotreload->current_datetime.second = datetime.second;
                    store_hotreload();
                });
                log_i(TAG_DatetimeService,
                      "Store datetime: %04d-%02d-%02d %02d:%02d:%02d",
                      (int)datetime.year,
                      (int)datetime.month,
                      (int)datetime.day,
                      (int)datetime.hour,
                      (int)datetime.minute,
                      (int)datetime.second);
            } else {
                log_w(TAG_DatetimeService,
                      "Invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
                      (int)datetime.year,
                      (int)datetime.month,
                      (int)datetime.day,
                      (int)datetime.hour,
                      (int)datetime.minute,
                      (int)datetime.second);
            }
        }
        if (datetime_are_valid || use_ntp) {
            wait_event_timed(datetime_service->events, &ulNotifiedValue, update_time_ms);
        } else {
            wait_event(datetime_service->events, &ulNotifiedValue);
        }
        log_d(TAG_DatetimeService, "new request, uxBits:0x%08X", (unsigned int)ulNotifiedValue);
    }

    log_w(TAG_DatetimeService, "Finish task");
    delete_current_task();
}
bool DatetimeService::EnableSntp() {
    bool enable;
    SAFETY_SETTINGS( //
        enable = settings.datetime.sntp_server_primary[0] != 0
              || settings.datetime.sntp_server_secondary[0] != 0; //
    );
    return enable;
}

void DatetimeService::SntpStateChanged() {
    raise_event(events, RESTART_SNTP_BIT);
}

void DatetimeService::time_sync_event_handler(void *arg, struct timeval *tv) {
    auto service = static_cast<DatetimeService *>(arg);
    log_i(TAG_DatetimeService, "time_sync_event_handler: %u", (unsigned int)tv->tv_sec);
    raise_event(service->events, STORE_BIT);
}

void DatetimeService::StartSntp() {
    CurrentSettings::datetime_settings dt_settings;

    SAFETY_SETTINGS(                     //
        dt_settings = settings.datetime; //
    );

    log_i(TAG_DatetimeService,
          "Start SNTP, serv_0:%s, serv_1:%s, tz:%s",
          dt_settings.sntp_server_primary,
          dt_settings.sntp_server_secondary,
          dt_settings.timezone);

    sntp_config_t config = {};
    config.event_handler_arg = this;
    config.primary_server = dt_settings.sntp_server_primary;
    config.secondary_server = dt_settings.sntp_server_secondary;
    config.time_sync_event_handler = &time_sync_event_handler;
    start_sntp(&config);

    setenv("TZ", dt_settings.timezone, 1);
    tzset();
}

void DatetimeService::StopSntp() {
    log_i(TAG_DatetimeService, "Stop SNTP");
    stop_sntp();
}

bool DatetimeService::SntpInProcess() {
    bool in_process = sntp_in_process();
    log_i(TAG_DatetimeService, "SntpInProcess :%u", in_process);
    return in_process;
}

void DatetimeService::GetCurrent(timeval *tv) {
    ERROR_CHECK(gettimeofday(tv, NULL) == 0 ? STATUS_OK : STATUS_FAIL);
}

void DatetimeService::SetCurrent(const timeval *tv) {
    settimeofday(tv, NULL);
}

int DatetimeService::GetCurrentSecond() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentSecond: %d", (int)tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentMinute: %d", (int)tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentHour: %d", (int)tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentDay: %d", (int)tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    int wdays_from_monday[] = { 7, 1, 2, 3, 4, 5, 6 };
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    int wday = wdays_from_monday[tm.tm_wday & 0x07];
    log_d(TAG_DatetimeService, "GetCurrentWeekday: %d", wday);
    return wday;
}

int DatetimeService::GetCurrentMonth() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentMonth: %d", (int)tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    log_d(TAG_DatetimeService, "GetCurrentYear: %d", (int)tm.tm_year);
    return tm.tm_year;
}

bool DatetimeService::ManualSet(Datetime *dt) {
    if (!ValidateDatetime(dt)) {
        log_w(TAG_DatetimeService,
              "ManualSet, invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
              (int)dt->year,
              (int)dt->month,
              (int)dt->day,
              (int)dt->hour,
              (int)dt->minute,
              (int)dt->second);
        return false;
    }

    timeval tv;
    GetCurrent(&tv);

    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    tm.tm_sec = dt->second;
    tm.tm_min = dt->minute;
    tm.tm_hour = dt->hour;
    tm.tm_mday = dt->day;
    tm.tm_mon = dt->month - 1;
    tm.tm_year = dt->year - DatetimeService::YearOffset;

    timeval new_tv = { mktime(&tm), tv.tv_usec };
    SetCurrent(&new_tv);
    raise_event(events, STORE_BIT);

    log_i(TAG_DatetimeService,
          "ManualSet: %04d-%02d-%02d %02d:%02d:%02d",
          (int)dt->year,
          (int)dt->month,
          (int)dt->day,
          (int)dt->hour,
          (int)dt->minute,
          (int)dt->second);
    return true;
}

void DatetimeService::Get(Datetime *dt) {
    timeval tv;
    GetCurrent(&tv);

    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    dt->second = tm.tm_sec;
    dt->minute = tm.tm_min;
    dt->hour = tm.tm_hour;
    dt->day = tm.tm_mday;
    dt->month = tm.tm_mon + 1;
    dt->year = tm.tm_year + DatetimeService::YearOffset;
}