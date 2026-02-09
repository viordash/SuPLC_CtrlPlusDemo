#pragma once

#include "buttons.h"
#include <stdint.h>
#include <unistd.h>

class button {
  private:
    const static uint32_t pressed_time_ms = 20;
    const static uint32_t long_pressed_time_ms = 500;

    uint64_t down_time;
    event_flag_t close_bit;
    event_flag_t open_bit;
    bool down;

  public:
    enum state {
        btNone = 0,
        btDown = 0x01,
        btShortPressed = 0x02,
        btPressed = 0x04,
        btLongPressed = 0x08,
    };
    const char *TAG;
    ButtonsPressType pressed_type;
    ButtonsPressType long_pressed_type;

    button(const char *tag,
           event_flag_t close_bit,
           event_flag_t open_bit,
           ButtonsPressType pressed_type,
           ButtonsPressType long_pressed_type);

    state handle(event_flag_t bits);
};