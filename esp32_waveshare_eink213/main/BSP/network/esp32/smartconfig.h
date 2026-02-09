#pragma once

#include "os.h"
#include <stdint.h>
#include <unistd.h>

typedef void (*smartconfig_started_event_handler_t)(void *arg);
typedef void (*smartconfig_disconnected_event_handler_t)(void *arg);
typedef void (*smartconfig_connected_event_handler_t)(void *arg);
typedef void (*smartconfig_scan_done_event_handler_t)(void *arg);
typedef void (*smartconfig_found_event_handler_t)(void *arg);
typedef void (*smartconfig_got_creds_event_handler_t)(void *arg,
                                                      const uint8_t ssid[32],
                                                      const uint8_t password[64],
                                                      const char *rvd_data);
typedef void (*smartconfig_done_event_handler_t)(void *arg);

typedef struct {
    smartconfig_started_event_handler_t started_event_handler;
    smartconfig_disconnected_event_handler_t disconnected_event_handler;
    smartconfig_connected_event_handler_t connected_event_handler;
    smartconfig_scan_done_event_handler_t scan_done_event_handler;
    smartconfig_found_event_handler_t found_event_handler;
    smartconfig_got_creds_event_handler_t got_creds_event_handler;
    smartconfig_done_event_handler_t done_event_handler;
    void *event_handler_arg;
} smartconfig_config_t;

void start_smartconfig(smartconfig_config_t *config);
void stop_smartconfig();