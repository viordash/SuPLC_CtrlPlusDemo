#pragma once

#include "Datetime/Datetime.h"
#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class RenderingService;
class WiFiService;
class Ladder;
class DatetimeService;
class DeviceIO;
class Controller {
  protected:
    static bool in_design;
    static WiFiService *wifi_service;
    static RenderingService *rendering_service;
    static DatetimeService *datetime_service;
    static ProcessWakeupService *process_wakeup_service;
    static LogicItemState network_continuation;

    friend DeviceIO;

  public:
    static event_t gpio_events;
    static void Start(event_t gpio_events,
                      WiFiService *wifi_service,
                      RenderingService *rendering_service,
                      DatetimeService *datetime_service,
                      ProcessWakeupService *process_wakeup_service);

    static void ProcessTask(void *parm);

    static bool
    RequestWakeupMs(const void *id, uint32_t delay_ms, ProcessWakeupRequestPriority priority);
    static void RemoveRequestWakeupMs(const void *id);
    static void RemoveExpiredWakeupRequests();
    static bool GetWakeupMs(uint32_t *delay_ms);

    static void WakeupProcessTask();

    static uint8_t ConnectToWiFiStation();
    static void DisconnectFromWiFiStation();

    static bool ManualSetSystemDatetime(Datetime *dt);
    static void GetSystemDatetime(Datetime *dt);
    static void RestartSntp();
    static void SetNetworkContinuation(LogicItemState state);
    static LogicItemState GetNetworkContinuation();

    static void UpdateUIViewTop(int32_t view_top_index);
    static void UpdateUISelected(int32_t selected_network);
    static void DesignStart();
    static void DesignEnd();
    static bool InDesign();
    static int32_t GetLastUpdatedUISelected();

    static void DoRender();
    static void DoDeferredRender();
};
