#pragma once

#include <stdint.h>
#include <unistd.h>

enum LogicItemState : uint8_t { lisPassive = 0, lisActive };

static inline bool ValidateLogicItemState(LogicItemState state) {
    if ((int8_t)state < (int8_t)LogicItemState::lisPassive) {
        return false;
    }
    if ((int8_t)state > (int8_t)LogicItemState::lisActive) {
        return false;
    }
    return true;
}