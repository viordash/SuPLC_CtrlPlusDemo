#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
}
#endif

typedef SemaphoreHandle_t mutex_handle_t;

static inline mutex_handle_t create_mutex() {
    return (mutex_handle_t)xSemaphoreCreateMutex();
}

static inline void delete_mutex(mutex_handle_t mutex) {
    vSemaphoreDelete(mutex);
}

static inline bool lock_mutex(mutex_handle_t mutex) {
    return xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE;
}
static inline bool lock_mutex_from_ISR(mutex_handle_t mutex) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult = xSemaphoreTakeFromISR(mutex, &xHigherPriorityTaskWoken);
    if (xResult == pdTRUE && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
    return xResult == pdTRUE;
}

static inline bool unlock_mutex(mutex_handle_t mutex) {
    return xSemaphoreGive(mutex) == pdTRUE;
}
static inline bool unlock_mutex_from_ISR(mutex_handle_t mutex) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult = xSemaphoreGiveFromISR(mutex, &xHigherPriorityTaskWoken);
    if (xResult == pdTRUE && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
    return xResult == pdTRUE;
}
