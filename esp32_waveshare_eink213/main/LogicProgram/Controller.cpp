

#include "LogicProgram/Controller.h"
#include "Datetime/DatetimeService.h"
#include "DeviceIO.h"
#include "Display/Common.h"
#include "Display/RenderingService.h"
#include "Display/display.h"
#include "Hotreload/hotreload_service.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include "WiFi/WiFiService.h"
#include "board.h"
#include "buttons.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

// #define STOP_PROCESS_TASK BIT1
#define STOP_RENDER_TASK BIT2
#define DO_RENDERING BIT3

extern CurrentSettings::device_settings settings;

bool Controller::in_design = false;
event_t Controller::gpio_events = INVALID_EVENT;
WiFiService *Controller::wifi_service = NULL;
RenderingService *Controller::rendering_service = NULL;
DatetimeService *Controller::datetime_service = NULL;
ProcessWakeupService *Controller::process_wakeup_service = NULL;
LogicItemState Controller::network_continuation = LogicItemState::lisPassive;

void Controller::Start(event_t gpio_events,
                       WiFiService *wifi_service,
                       RenderingService *rendering_service,
                       DatetimeService *datetime_service,
                       ProcessWakeupService *process_wakeup_service) {
    Controller::gpio_events = gpio_events;
    Controller::wifi_service = wifi_service;
    Controller::rendering_service = rendering_service;
    Controller::datetime_service = datetime_service;
    Controller::process_wakeup_service = process_wakeup_service;

    log_i(TAG_Controller, "start, gpio_events:%u", (unsigned int)(uintptr_t)gpio_events);

    task_handle_t task_handle = create_task(ProcessTask,
                                            "ctrl_task",
                                            Controller_Task_Stack_size,
                                            NULL,
                                            task_priority_t::tp_High);
    subscribe_to_event(Controller::gpio_events, task_handle);
}

void Controller::ProcessTask(void *parm) {
    (void)parm;

    log_i(TAG_Controller, "start process task");

    network_continuation = LogicItemState::lisPassive;

    Ladder ladder;

    ladder.Load();
    if (hotreload->is_hotstart) {
        ladder.SetViewTopIndex(hotreload->view_top_index);
        ladder.SetSelectedNetworkIndex(hotreload->selected_network);
    }
    ladder.AtLeastOneNetwork();

    rendering_service->Start(&ladder);

    const uint32_t first_iteration_delay = 0;
    Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                first_iteration_delay,
                                ProcessWakeupRequestPriority::pwrp_Critical);
    while (true) {
        event_flags_t flags;
        uint32_t delay_ms;
        if (Controller::GetWakeupMs(&delay_ms)) {
            wait_event_timed(Controller::gpio_events, &flags, delay_ms);
        } else {
            wait_event(Controller::gpio_events, &flags);
        }

        Controller::RemoveExpiredWakeupRequests();

        log_d(TAG_Controller, "flags:0x%08X", (unsigned int)flags);
        bool inputs_changed = (flags & INPUTS_EVENTS_BITS) != 0;
        bool buttons_changed = (flags & BUTTONS_EVENTS_BITS) != 0;

        static_assert((INPUTS_EVENTS_BITS & BUTTONS_EVENTS_BITS) == 0
                          && (INPUTS_EVENTS_BITS & TOUCHSCREEN_EVENTS_BITS) == 0
                          && (BUTTONS_EVENTS_BITS & TOUCHSCREEN_EVENTS_BITS) == 0,
                      "Intersection detected between INPUTS_EVENTS_BITS or BUTTONS_EVENTS_BITS or "
                      "TOUCHSCREEN_EVENTS_BITS");
        bool do_render = inputs_changed;

        if (buttons_changed) {
            ButtonsPressType pressed_button = handle_buttons(flags);
            log_d(TAG_Controller, "buttons_changed, pressed_button:%u", pressed_button);
            switch (pressed_button) {
                case ButtonsPressType::UP_PRESSED:
                    ladder.HandleButtonUp();
                    do_render = true;
                    break;
                case ButtonsPressType::UP_LONG_PRESSED:
                    ladder.HandleButtonPageUp();
                    do_render = true;
                    break;
                case ButtonsPressType::DOWN_PRESSED:
                    ladder.HandleButtonDown();
                    do_render = true;
                    break;
                case ButtonsPressType::DOWN_LONG_PRESSED:
                    ladder.HandleButtonPageDown();
                    do_render = true;
                    break;
                case ButtonsPressType::SELECT_PRESSED:
                    ladder.HandleButtonSelect();
                    do_render = true;
                    break;
                case ButtonsPressType::SELECT_LONG_PRESSED:
                    ladder.HandleButtonOption();
                    do_render = true;
                    break;
                default:
                    break;
            }
        }

        bool looped_actions = false;
        auto cycle_start_time = timer_get_time_us();
        const uint64_t max_cycle_ms = SystemOS_Tick_Period_ms * 2;
        auto next_time = cycle_start_time + (max_cycle_ms * 1000);
        bool expired;
        do {
            DeviceIO::FetchIOValues();
            bool any_changes_in_actions = ladder.DoAction();
            DeviceIO::CommitChanges();
            if (!any_changes_in_actions) {
                break;
            }
            int64_t timespan = next_time - timer_get_time_us();
            expired = timespan <= (SystemOS_Tick_Period_ms / 2) * 1000;
            do_render = true;
            looped_actions = expired;
        } while (!expired);

        if (looped_actions) {
            log_d(TAG_Controller, "looped actions");
            Controller::RemoveRequestWakeupMs((void *)Controller::ProcessTask);
            const uint32_t loop_break_ms = SystemOS_Tick_Period_ms * 2;
            Controller::RequestWakeupMs((void *)Controller::ProcessTask,
                                        loop_break_ms,
                                        ProcessWakeupRequestPriority::pwrp_Critical);
        }

        if (do_render || Controller::in_design) {
            Controller::DoRender();
        }
    }

    log_i(TAG_Controller, "stop process task");
    delete_current_task();
}

bool Controller::RequestWakeupMs(const void *id,
                                 uint32_t delay_ms,
                                 ProcessWakeupRequestPriority priority) {
    if (Controller::process_wakeup_service == NULL) {
        return false;
    }
    return process_wakeup_service->Request(id, delay_ms, priority);
}

void Controller::RemoveRequestWakeupMs(const void *id) {
    if (Controller::process_wakeup_service == NULL) {
        return;
    }
    process_wakeup_service->RemoveRequest(id);
}

void Controller::RemoveExpiredWakeupRequests() {
    if (Controller::process_wakeup_service == NULL) {
        return;
    }
    process_wakeup_service->RemoveExpired();
}

bool Controller::GetWakeupMs(uint32_t *delay_ms) {
    if (Controller::process_wakeup_service == NULL) {
        return false;
    }
    return process_wakeup_service->Get(delay_ms);
}

void Controller::WakeupProcessTask() {
    raise_wakeup_event(Controller::gpio_events);
}

uint8_t Controller::ConnectToWiFiStation() {
    if (Controller::wifi_service != NULL) {
        return Controller::wifi_service->ConnectToStation();
    }
    return LogicElement::MinValue;
}

void Controller::DisconnectFromWiFiStation() {
    if (Controller::wifi_service != NULL) {
        Controller::wifi_service->DisconnectFromStation();
    }
}

bool Controller::ManualSetSystemDatetime(Datetime *dt) {
    if (Controller::datetime_service == NULL) {
        return false;
    }
    return Controller::datetime_service->ManualSet(dt);
}

void Controller::GetSystemDatetime(Datetime *dt) {
    if (Controller::datetime_service == NULL) {
        return;
    }
    Controller::datetime_service->Get(dt);
}

void Controller::RestartSntp() {
    if (Controller::datetime_service == NULL) {
        return;
    }
    Controller::datetime_service->SntpStateChanged();
}

void Controller::SetNetworkContinuation(LogicItemState state) {
    network_continuation = state;
}

LogicItemState Controller::GetNetworkContinuation() {
    return network_continuation;
}

void Controller::UpdateUIViewTop(int32_t view_top_index) {
    SAFETY_HOTRELOAD({
        hotreload->view_top_index = view_top_index;
        store_hotreload();
    });
    log_d(TAG_Controller, "UpdateUIViewTop %d", (int)view_top_index);
    Controller::WakeupProcessTask();
}

void Controller::UpdateUISelected(int32_t selected_network) {
    SAFETY_HOTRELOAD({
        hotreload->selected_network = selected_network;
        store_hotreload();
    });
    log_d(TAG_Controller, "UpdateUISelected %d", (int)selected_network);
}

int32_t Controller::GetLastUpdatedUISelected() {
    return hotreload->selected_network;
}

void Controller::DesignStart() {
    Controller::in_design = true;
    Controller::WakeupProcessTask();
    log_i(TAG_Controller, "DesignStart");
}

void Controller::DesignEnd() {
    Controller::in_design = false;
    log_i(TAG_Controller, "DesignEnd");
}

bool Controller::InDesign() {
    return Controller::in_design;
}

void Controller::DoRender() {
    if (Controller::rendering_service == NULL) {
        return;
    }
    rendering_service->Do();
}

void Controller::DoDeferredRender() {
    if (Controller::rendering_service == NULL) {
        return;
    }
    rendering_service->DoDeferred();
}