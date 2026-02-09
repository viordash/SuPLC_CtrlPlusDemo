#pragma once

#include "BSP/os/esp32/tasks.h"
#include <stdint.h>
#include <unistd.h>

typedef void *event_t;
#define INVALID_EVENT NULL

typedef uint32_t event_flags_t;
typedef uint32_t event_flag_t;

event_t create_event();
void delete_event(event_t event);
void subscribe_to_event(event_t event, task_handle_t task);
void unsubscribe_from_event(event_t event, task_handle_t task);
void raise_wakeup_event(event_t event);
void raise_event(event_t event, event_flags_t flags);
void raise_wakeup_event_from_ISR(event_t event);
void raise_event_from_ISR(event_t event, event_flags_t flags);
bool wait_event(event_t event, event_flags_t *notified_flags);
bool wait_event_timed(event_t event, event_flags_t *notified_flags, uint32_t timeout_ms);