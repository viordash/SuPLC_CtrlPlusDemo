#pragma once

#include "os.h"
#include <stdint.h>
#include <unistd.h>

typedef enum { //
    NOTHING_PRESSED,
    UP_PRESSED,
    UP_LONG_PRESSED,
    DOWN_PRESSED,
    DOWN_LONG_PRESSED,
    SELECT_PRESSED,
    SELECT_LONG_PRESSED
} ButtonsPressType;

ButtonsPressType handle_buttons(event_flags_t flags);
