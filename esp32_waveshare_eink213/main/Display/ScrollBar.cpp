#include "Display/ScrollBar.h"
#include "Display/display.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

static const char *TAG_ScrollBar = "ScrollBar";

IRAM_ATTR void
ScrollBar::Render(FrameBuffer *fb, size_t count, size_t viewport_count, size_t view_topindex) {
    bool nothing_to_scroll = count <= viewport_count;
    if (nothing_to_scroll) {
        return;
    }
    uint16_t x = SCROLLBAR_LEFT;

    uint16_t height = ((size_t)SCROLLBAR_HEIGHT * viewport_count) / count;
    if (height < MAX((SCROLLBAR_HEIGHT / 16), 3)) {
        height = MAX((SCROLLBAR_HEIGHT / 16), 3);
    }

    uint16_t y;

    uint16_t moving_area = SCROLLBAR_HEIGHT - height;
    uint16_t steps = count - viewport_count;
    if (moving_area >= steps) {
        uint16_t step_height = moving_area / steps;
        uint16_t y_offset = view_topindex * step_height;
        uint16_t round_tail = moving_area - (step_height * steps);
        height += round_tail;
        y = SCROLLBAR_TOP + y_offset;
    } else {
        uint16_t step_mul = (steps * 10) / moving_area;
        uint16_t step_div = (steps * step_mul) / moving_area;
        uint16_t y_offset = (view_topindex * step_mul) / step_div;
        y = SCROLLBAR_TOP + y_offset;
    }

    log_d(TAG_ScrollBar,
          "Render: x:%u, y:%u, height:%u, top:%u, count:%u, viewport_count:%u, view_topindex:%u",
          (unsigned int)x,
          (unsigned int)y,
          (unsigned int)height,
          (unsigned int)SCROLLBAR_TOP,
          (unsigned int)count,
          (unsigned int)viewport_count,
          (unsigned int)view_topindex);

    ASSERT(draw_vert_line(fb, x, y, height));
}
