#pragma once

#include <stdint.h>
#include <unistd.h>

typedef void (*sntp_time_sync_event_handler_t)(void *arg, struct timeval *tv);

typedef struct {
    const char *primary_server;
    const char *secondary_server;
    sntp_time_sync_event_handler_t time_sync_event_handler;
    void *event_handler_arg;
} sntp_config_t;

void start_sntp(sntp_config_t *config);
void stop_sntp();
bool sntp_in_process();