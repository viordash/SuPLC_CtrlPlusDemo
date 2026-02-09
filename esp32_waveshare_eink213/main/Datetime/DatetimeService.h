#pragma once

#include "Datetime/Datetime.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

class DatetimeService {
  protected:
    event_t events;

    static void Task(void *parm);
    static void time_sync_event_handler(void *arg, struct timeval *tv);
    virtual void GetCurrent(timeval *tv);
    virtual void SetCurrent(const timeval *tv);
    bool EnableSntp();
    void StartSntp();
    void StopSntp();
    bool SntpInProcess();

  public:
    static const int STORE_BIT = BIT1;
    static const int RESTART_SNTP_BIT = BIT2;

    const static int YearOffset = 1900;
    DatetimeService();
    virtual ~DatetimeService();

    void Start();

    void SntpStateChanged();

    int GetCurrentSecond();
    int GetCurrentMinute();
    int GetCurrentHour();
    int GetCurrentDay();
    int GetCurrentWeekday();
    int GetCurrentMonth();
    int GetCurrentYear();

    bool ManualSet(Datetime *dt);
    void Get(Datetime *dt);
};
