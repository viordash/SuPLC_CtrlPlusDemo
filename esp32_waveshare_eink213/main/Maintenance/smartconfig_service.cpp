
#include "smartconfig_service.h"
#include "Settings/settings.h"
#include "board.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "smartconfig";

static const uint32_t timeout_ms = 600000;

extern CurrentSettings::device_settings settings;

static struct {
    event_t event = INVALID_EVENT;
} service;

static const int STATUS_START_BIT = BIT2;
static const int STATUS_STARTED_BIT = BIT3;
static const int STATUS_DISCONNECTED_BIT = BIT4;
static const int STATUS_GOT_IP_BIT = BIT5;
static const int STATUS_SCAN_DONE_BIT = BIT6;
static const int STATUS_FOUND_CHANNEL_BIT = BIT7;
static const int STATUS_GOT_CREDS_BIT = BIT8;
static const int STATUS_COMPLETED_BIT = BIT9;
static const int STATUS_ERROR_BIT = BIT10;

void started_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_STARTED_BIT);
}
void disconnected_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_DISCONNECTED_BIT);
}
void connected_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_GOT_IP_BIT);
}
void scan_done_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_SCAN_DONE_BIT);
}
void found_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_FOUND_CHANNEL_BIT);
}
void got_creds_event_handler(void *arg,
                             const uint8_t ssid[32],
                             const uint8_t password[64],
                             const char *rvd_data) {
    (void)arg;
    (void)ssid;
    (void)password;
    (void)rvd_data;
    (void)rvd_data;
    raise_event(service.event, STATUS_GOT_CREDS_BIT);
}
void done_event_handler(void *arg) {
    (void)arg;
    raise_event(service.event, STATUS_COMPLETED_BIT);
}

static void start_process() {
    log_w(TAG, "Start process");

    smartconfig_config_t config = {};
    config.started_event_handler = &started_event_handler;
    config.disconnected_event_handler = &disconnected_event_handler;
    config.connected_event_handler = &connected_event_handler;
    config.scan_done_event_handler = &scan_done_event_handler;
    config.found_event_handler = &found_event_handler;
    config.got_creds_event_handler = &got_creds_event_handler;
    config.done_event_handler = &done_event_handler;
    config.event_handler_arg = &service;
    start_smartconfig(&config);

    bool connected = false;
    while (true) {
        event_flags_t flags;
        bool timeout = !wait_event_timed(service.event, &flags, timeout_ms);

        log_i(TAG, "process, flags:0x%08X", (unsigned int)flags);

        if (timeout) {
            log_i(TAG, "timeout");
            return;
        }

        if (flags & STATUS_GOT_IP_BIT) {
            log_i(TAG, "WiFi Connected to ap");
            connected = true;
        }

        if (flags & STATUS_COMPLETED_BIT) {
            log_i(TAG, "smartconfig over");
            break;
        }

        if (flags & STATUS_DISCONNECTED_BIT) {
            log_i(TAG, "Restarting smartconfig");
            connected = false;
            stop_smartconfig();
            start_smartconfig(&config);
        }
    }

    if (connected) {
        char ssid[sizeof(wifi_station_config_t::ssid) + 1] = {};
        char pwd[sizeof(wifi_station_config_t::password) + 1] = {};
        get_wifi_station_creds(ssid, sizeof(ssid), pwd, sizeof(pwd));

        SAFETY_SETTINGS( //
            memcpy(settings.wifi_station.ssid, ssid, sizeof(settings.wifi_station.ssid));
            memcpy(settings.wifi_station.password,
                   pwd,
                   sizeof(settings.wifi_station.password));
            store_settings(); //
        );
        log_i(TAG, "store wifi settings, ssid:%s, pwd:%s", ssid, pwd);
    } else {
        raise_event(service.event, STATUS_ERROR_BIT);
    }

    stop_smartconfig();
    log_w(TAG, "Finish process");
}

static void task(void *parm) {
    (void)parm;
    log_i(TAG, "Start task");

    raise_event(service.event, STATUS_START_BIT);
    start_process();

    log_w(TAG, "Finish task");
    delete_current_task();
}

void start_smartconfig() {
    log_i(TAG, "start_smartconfig");
    service.event = create_event();

    task_handle_t task_handle = create_task(task,
                                            "smartconfig_task",
                                            Smartconfig_Task_Stack_size,
                                            NULL,
                                            task_priority_t::tp_Middle);

    task_handle_t curr_task = get_current_task_handle();
    subscribe_to_event(service.event, curr_task);
    subscribe_to_event(service.event, task_handle);
}

void finish_smartconfig() {
    delete_event(service.event);
    log_i(TAG, "finish_smartconfig");
}

enum SmartconfigStatus smartconfig_status() {
    if (service.event == INVALID_EVENT) {
        return scs_Error;
    }

    event_flags_t flags;
    wait_event(service.event, &flags);

    if (flags & STATUS_START_BIT) {
        return scs_Start;
    }
    if (flags & STATUS_STARTED_BIT) {
        return scs_Started;
    }
    if (flags & STATUS_DISCONNECTED_BIT) {
        return scs_Disconnected;
    }
    if (flags & STATUS_GOT_IP_BIT) {
        return scs_GotIP;
    }
    if (flags & STATUS_SCAN_DONE_BIT) {
        return scs_ScanDone;
    }
    if (flags & STATUS_FOUND_CHANNEL_BIT) {
        return scs_FoundChannel;
    }
    if (flags & STATUS_GOT_CREDS_BIT) {
        return scs_GotCreds;
    }
    if (flags & STATUS_COMPLETED_BIT) {
        return scs_Completed;
    }
    return scs_Error;
}