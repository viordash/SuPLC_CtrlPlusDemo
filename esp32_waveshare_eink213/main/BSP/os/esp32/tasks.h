#pragma once

#include "BSP/os/esp32/system.h"
#include <stdint.h>
#include <unistd.h>

typedef TaskHandle_t task_handle_t;
typedef void (*task_function_t)(void *arg);
typedef uint32_t stack_depth_t;
typedef enum { //
    tp_Idle = 0,
    tp_Low = 5,
    tp_Middle = 10,
    tp_High = 15,
    tp_Critical = 20
} task_priority_t;

static inline task_handle_t create_task(task_function_t function,
                                        const char *const name,
                                        stack_depth_t stack_depth,
                                        void *const params,
                                        task_priority_t priority) {
    TaskHandle_t task_handle;
    ESP_ERROR_CHECK(
        xTaskCreate(function, name, stack_depth, params, (UBaseType_t)priority, &task_handle)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);
    return (task_handle_t)task_handle;
}

static inline void delete_current_task() {
    vTaskDelete((TaskHandle_t)NULL);
}

static inline task_handle_t get_current_task_handle() {
    return (task_handle_t)xTaskGetCurrentTaskHandle();
};

static inline void sleep_ms(const uint32_t ms) {
    vTaskDelay((ms + (portTICK_PERIOD_MS / 2)) / portTICK_PERIOD_MS);
}