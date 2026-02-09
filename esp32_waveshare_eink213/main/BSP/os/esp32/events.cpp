
#include "os.h"
#include <algorithm>
#include <atomic>
#include <stdint.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

static const char *TAG_events = "os/events";

static std::unordered_map<event_t, std::vector<task_handle_t>> events;
static std::atomic<size_t> task_id{ 0 };

event_t create_event() {
    auto res = events.emplace(
        std::make_pair(reinterpret_cast<event_t>(task_id++), std::vector<task_handle_t>()));

    auto inserted_event = res.first;
    auto evt = inserted_event->first;
    auto &tasks = inserted_event->second;
    log_i(TAG_events, "create_event, is_inserted:%u, evt:%p, tasks:%p", res.second, evt, &tasks);
    return evt;
}

void delete_event(event_t event) {
    auto removed = events.erase(event);
    log_i(TAG_events, "delete_event, evt:%p, removed:%zu", event, removed);
}

void subscribe_to_event(event_t event, task_handle_t task) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "subscribe_to_event, not found evt:%p, task:%p", event, task);
        return;
    }
    auto &tasks = evt->second;
    tasks.push_back(task);
    log_i(TAG_events,
          "subscribe_to_event, evt:%p, task:%p, tasks:%p (%zu)",
          event,
          task,
          &tasks,
          tasks.size());
}

void unsubscribe_from_event(event_t event, task_handle_t task) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "unsubscribe_from_event, not found evt:%p, task:%p", event, task);
        return;
    }
    auto &tasks = evt->second;
    auto it = std::find(tasks.begin(), tasks.end(), task);
    if (it != tasks.end()) {
        tasks.erase(it);
        log_i(TAG_events,
              "unsubscribe_from_event, evt:%p, task:%p, tasks:%p (%zu)",
              event,
              task,
              &tasks,
              tasks.size());
    }
}

void raise_wakeup_event(event_t event) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "raise_wakeup_event, not found evt:%p", event);
        return;
    }
    auto &tasks = evt->second;
    log_d(TAG_events, "raise_wakeup_event, evt:%p, tasks:%p (%zu)", event, &tasks, tasks.size());

    for (auto &task_handle : tasks) {
        xTaskNotifyIndexed(task_handle,
                           reinterpret_cast<UBaseType_t>(event),
                           0,
                           eNotifyAction::eNoAction);
        log_d(TAG_events, "raise_wakeup_event, evt:%p, task:%p", event, task_handle);
    }
}

void raise_event(event_t event, event_flags_t flags) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "raise_event, not found evt:%p", event);
        return;
    }
    auto &tasks = evt->second;
    log_d(TAG_events, "raise_event, evt:%p, tasks:%p (%zu)", event, &tasks, tasks.size());

    for (auto &task_handle : tasks) {
        xTaskNotifyIndexed(task_handle,
                           reinterpret_cast<UBaseType_t>(event),
                           reinterpret_cast<uint32_t>(flags),
                           eNotifyAction::eSetBits);
        log_d(TAG_events,
              "raise_event, evt:%p, task:%p, flags:0x%08X",
              event,
              task_handle,
              (unsigned int)flags);
    }
}

void raise_wakeup_event_from_ISR(event_t event) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        return;
    }
    auto &tasks = evt->second;
    for (auto &task_handle : tasks) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        BaseType_t xResult = xTaskNotifyIndexedFromISR(task_handle,
                                                       reinterpret_cast<UBaseType_t>(event),
                                                       0,
                                                       eNotifyAction::eSetBits,
                                                       &xHigherPriorityTaskWoken);
        if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR();
        }
    }
}

void raise_event_from_ISR(event_t event, event_flags_t flags) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        return;
    }
    auto &tasks = evt->second;
    for (auto &task_handle : tasks) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        BaseType_t xResult = xTaskNotifyIndexedFromISR(task_handle,
                                                       reinterpret_cast<UBaseType_t>(event),
                                                       reinterpret_cast<uint32_t>(flags),
                                                       eNotifyAction::eSetBits,
                                                       &xHigherPriorityTaskWoken);
        if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR();
        }
    }
}

bool wait_event(event_t event, event_flags_t *notified_flags) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "wait_event, not found evt:%p", event);
        *notified_flags = 0;
        return false;
    }

    log_d(TAG_events, "wait_event, evt:%p", event);
    uint32_t flags = 0;
    if (xTaskNotifyWaitIndexed(reinterpret_cast<UBaseType_t>(event),
                               0,
                               0xFFFFFFFF,
                               &flags,
                               portMAX_DELAY)
        != pdTRUE) {
        log_d(TAG_events, "wait_event, timeout evt:%p", event);
        *notified_flags = 0;
        return false;
    }
    *notified_flags = flags;
    log_d(TAG_events,
          "wait_event, evt:%p, value:0x%08X, task:%p",
          event,
          (unsigned int)flags,
          get_current_task_handle());
    return true;
}

bool wait_event_timed(event_t event, event_flags_t *notified_flags, uint32_t timeout_ms) {
    auto evt = events.find(event);
    if (evt == events.end()) {
        log_e(TAG_events, "wait_event_timed, not found evt:%p", event);
        *notified_flags = 0;
        return false;
    }

    TickType_t wait_ticks = ((uint64_t)timeout_ms + (portTICK_PERIOD_MS / 2)) / portTICK_PERIOD_MS;
    log_d(TAG_events,
          "wait_event_timed, evt:%p, timeout_ms:%u, wait_ticks:%u",
          event,
          (unsigned int)timeout_ms,
          (unsigned int)wait_ticks);
    uint32_t flags = 0;
    if (xTaskNotifyWaitIndexed(reinterpret_cast<UBaseType_t>(event),
                               0,
                               0xFFFFFFFF,
                               &flags,
                               wait_ticks)
        != pdTRUE) {
        log_d(TAG_events, "wait_event_timed, timeout evt:%p", event);
        *notified_flags = 0;
        return false;
    }
    *notified_flags = flags;
    log_d(TAG_events,
          "wait_event_timed, evt:%p, value:0x%08X, task:%p, timeout_ms:%u",
          event,
          (unsigned int)flags,
          get_current_task_handle(),
          (unsigned int)timeout_ms);
    return true;
}
