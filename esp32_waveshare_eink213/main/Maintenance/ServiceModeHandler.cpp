
#include "Maintenance/ServiceModeHandler.h"
#include "Display/Common.h"
#include "Display/ListBox.h"
#include "Display/display.h"
#include "Settings/settings.h"
#include "buttons.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler = "ServiceMode";

#define EXPECTED_BUTTONS                                                                           \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

void ServiceModeHandler::Start(t_board *board) {
    char buffer[64];
    Mode mode = Mode::sm_SmartConfig;

    sprintf(buffer, "v%08X.%u", (unsigned int)DEVICE_SETTINGS_VERSION, (unsigned int)BUILD_NUMBER);
    ListBox listBox(buffer);
    listBox.Insert(0, "Smart config");
    listBox.Insert(1, "Backup logic");
    listBox.Insert(2, "Restore logic");
    listBox.Insert(3, "Reset to default");
    listBox.Select(mode);

    task_handle_t curr_task = get_current_task_handle();
    subscribe_to_event(board->gpio_events, curr_task);
    subscribe_to_event(board->display_events, curr_task);

    while (true) {
        auto fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        event_flags_t flags;
        bool timeout = !wait_event_timed(board->gpio_events, &flags, service_mode_timeout_ms);

        log_d(TAG_ServiceModeHandler, "flags:0x%08X", (unsigned int)flags);

        if (timeout) {
            log_i(TAG_ServiceModeHandler, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(flags);
        log_d(TAG_ServiceModeHandler, "buttons_changed, pressed_button:%u", pressed_button);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
            case ButtonsPressType::UP_LONG_PRESSED:
                mode = ChangeModeToPrev(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
            case ButtonsPressType::DOWN_LONG_PRESSED:
                mode = ChangeModeToNext(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
            case ButtonsPressType::SELECT_LONG_PRESSED:
                Execute(board->gpio_events, mode);
                unsubscribe_from_event(board->display_events, curr_task);
                unsubscribe_from_event(board->gpio_events, curr_task);
                return;
            default:
                break;
        }
    }
}

ServiceModeHandler::Mode ServiceModeHandler::ChangeModeToPrev(ServiceModeHandler::Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            mode = Mode::sm_ResetToDefault;
            break;
        case Mode::sm_BackupLogic:
            mode = Mode::sm_SmartConfig;
            break;
        case Mode::sm_RestoreLogic:
            mode = Mode::sm_BackupLogic;
            break;
        case Mode::sm_ResetToDefault:
            mode = Mode::sm_RestoreLogic;
            break;
    }
    return mode;
}

ServiceModeHandler::Mode ServiceModeHandler::ChangeModeToNext(ServiceModeHandler::Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            mode = Mode::sm_BackupLogic;
            break;
        case Mode::sm_BackupLogic:
            mode = Mode::sm_RestoreLogic;
            break;
        case Mode::sm_RestoreLogic:
            mode = Mode::sm_ResetToDefault;
            break;
        case Mode::sm_ResetToDefault:
            mode = Mode::sm_SmartConfig;
            break;
    }
    return mode;
}

void ServiceModeHandler::Execute(event_t gpio_events, Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            SmartConfig(gpio_events);
            break;
        case Mode::sm_BackupLogic:
            Backup(gpio_events);
            break;
        case Mode::sm_RestoreLogic:
            Restore(gpio_events);
            break;
        case Mode::sm_ResetToDefault:
            ResetData(gpio_events);
            break;
    }
}

void ServiceModeHandler::ShowStatus(event_t gpio_events,
                                    bool success,
                                    const char *success_message,
                                    const char *error_message) {
    uint8_t x = 1;
    uint8_t y = 1;

    FontLarge text_font;

    FrameBuffer *fb = begin_render();
    ASSERT(text_font.DrawText(fb,
                              x,
                              y + text_font.GetHeight() * 1,
                              success //
                                  ? success_message
                                  : error_message)
           > 0);
    ASSERT(text_font.DrawText(fb, x, y + text_font.GetHeight() * 2, "Press SELECT to exit") > 0);
    fb->has_changes = true;
    end_render(fb);

    event_flags_t flags;
    wait_event(gpio_events, &flags);
}