#include "Display/Common.h"
#include "Display/LogsList.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "Maintenance/ServiceModeHandler.h"
#include "Maintenance/backups_storage.h"
#include "Settings/settings.h"
#include "board.h"
#include "buttons.h"
#include "os.h"
#include "partitions.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler_Reset = "ServiceMode.Reset";

const char *ServiceModeHandler::reset_data_names[4] = { "Settings",
                                                        "Ladder program",
                                                        "Backups",
                                                        "Factory reset" };

void ServiceModeHandler::ResetData(event_t gpio_events) {
    log_i(TAG_ServiceModeHandler_Reset, "execute");

    ResetMode mode = ResetMode::rd_Settings;

    ListBox listBox("Reset data");
    for (size_t i = 0; i < sizeof(reset_data_names) / sizeof(reset_data_names[0]); i++) {
        listBox.Insert(i, reset_data_names[i]);
    }

    listBox.Select(mode);

    bool success = false;
    bool error = false;
    while (!success && !error) {
        auto fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        event_flags_t flags;
        bool timeout = !wait_event_timed(gpio_events, &flags, service_mode_timeout_ms);

        if (timeout) {
            log_i(TAG_ServiceModeHandler_Reset, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(flags);
        log_d(TAG_ServiceModeHandler_Reset,
              "buttons_changed, pressed_button:%u, bits:0x%08X",
              (unsigned int)pressed_button,
              (unsigned int)flags);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
            case ButtonsPressType::UP_LONG_PRESSED:
                mode = ChangeResetModeToPrev(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
            case ButtonsPressType::DOWN_LONG_PRESSED:
                mode = ChangeResetModeToNext(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
            case ButtonsPressType::SELECT_LONG_PRESSED:
                success = DoResetData(gpio_events, mode);
                error = !success;
                break;
            default:
                break;
        }
    }
    ShowStatus(gpio_events, success, "Completed!", "Canceled!");
    restart_system();
}

ServiceModeHandler::ResetMode
ServiceModeHandler::ChangeResetModeToPrev(ServiceModeHandler::ResetMode mode) {
    switch (mode) {
        case ResetMode::rd_Settings:
            mode = ResetMode::rd_FactoryReset;
            break;
        case ResetMode::rd_Ladder:
            mode = ResetMode::rd_Settings;
            break;
        case ResetMode::rd_Backups:
            mode = ResetMode::rd_Ladder;
            break;
        case ResetMode::rd_FactoryReset:
            mode = ResetMode::rd_Backups;
            break;
    }
    return mode;
}

ServiceModeHandler::ResetMode
ServiceModeHandler::ChangeResetModeToNext(ServiceModeHandler::ResetMode mode) {
    switch (mode) {
        case ResetMode::rd_Settings:
            mode = ResetMode::rd_Ladder;
            break;
        case ResetMode::rd_Ladder:
            mode = ResetMode::rd_Backups;
            break;
        case ResetMode::rd_Backups:
            mode = ResetMode::rd_FactoryReset;
            break;
        case ResetMode::rd_FactoryReset:
            mode = ResetMode::rd_Settings;
            break;
    }
    return mode;
}

bool ServiceModeHandler::DoResetData(event_t gpio_events, ServiceModeHandler::ResetMode mode) {
    log_i(TAG_ServiceModeHandler_Reset, "DoResetData, mode:%d", (int)mode);

    uint8_t x = 1;
    uint8_t y = 1;

    FontLarge text_font;

    FrameBuffer *fb = begin_render();
    ASSERT(text_font.DrawText(fb, x, y + text_font.GetHeight() * 1, "Reset:") > 0);
    ASSERT(text_font.DrawText(fb, x, y + text_font.GetHeight() * 2, reset_data_names[mode]) > 0);
    ASSERT(text_font.DrawText(fb, x, y + text_font.GetHeight() * 3, "Press UP to continue") > 0);
    fb->has_changes = true;
    end_render(fb);

    event_flags_t flags;
    wait_event(gpio_events, &flags);

    log_i(TAG_ServiceModeHandler_Reset, "flags:0x%08X", (unsigned int)flags);

    bool button_up_released = (flags & BUTTON_UP_IO_OPEN) != 0;
    if (!button_up_released) {
        log_i(TAG_ServiceModeHandler_Reset, "Canceled");
        return false;
    }

    switch (mode) {
        case ResetMode::rd_Settings:
            delete_settings();
            break;
        case ResetMode::rd_Ladder:
            Ladder::DeleteStorage();
            break;
        case ResetMode::rd_Backups:
            DeleteBackupFiles(max_backup_files);
            break;
        case ResetMode::rd_FactoryReset:
            delete_settings();
            Ladder::DeleteStorage();
            DeleteBackupFiles(max_backup_files);
            break;
    }
    return true;
}