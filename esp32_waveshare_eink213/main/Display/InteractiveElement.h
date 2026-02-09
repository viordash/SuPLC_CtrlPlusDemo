#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class InteractiveElement {
  public:
  protected:
    enum State { csReleased = 0, csPressed, csMoved };
    explicit InteractiveElement();

    Rect interactive_area;
    State interactive_state;

    void SetInteractiveArea(const Point *top_left, const Point *bottom_right);
    void DetermineInteractive();
};
