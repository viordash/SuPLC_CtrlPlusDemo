#include "Display/InteractiveElement.h"
#include "Display/Common.h"
#include "Display/display.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUPPORT_INTERACTIVE

static const char *TAG_InteractiveElement = "InteractiveElement";

InteractiveElement::InteractiveElement() {
    interactive_area = {};
    interactive_state = State::csReleased;
}

void InteractiveElement::SetInteractiveArea(const Point *top_left, const Point *bottom_right) {
    interactive_area.top_left = *top_left;
    interactive_area.bottom_right = *bottom_right;
}

void InteractiveElement::DetermineInteractive() {
    Point coord;
    interactive_state = State::csReleased;
    if (get_touch_screen_pressed(&coord)) {
        bool in_area =
            coord.x >= interactive_area.top_left.x && coord.x <= interactive_area.bottom_right.x
            && coord.y >= interactive_area.top_left.y && coord.y <= interactive_area.bottom_right.y;
        if (in_area) {
            interactive_state = State::csPressed;
        }
        log_d(TAG_InteractiveElement,
              "IsPressed: x: %u, y: %u, in_area: %u",
              coord.x,
              coord.y,
              in_area);
    }
}

#endif //SUPPORT_INTERACTIVE