#pragma once

#include "os.h"

// Display resolution
#define DISPLAY_ID "ssd1680"
#define DISPLAY_WIDTH DISPLAY_HW_HEIGHT
#define DISPLAY_HEIGHT DISPLAY_HW_WIDTH
#define DISPLAY_HW_WIDTH 122
#define DISPLAY_HW_HEIGHT 250
#define DISPLAY_HEIGHT_IN_BYTES ((DISPLAY_HEIGHT + 7) / 8)
#define DISPLAY_MIN_PERIOD_RENDER_MS 2000

#define DISPLAY_PIXELS_OFF 0xFF
#define FRAMEBUFFER_STUB NULL

typedef struct {
    uint8_t buffer[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH];
    bool has_changes;
    int32_t view_offset;
    uint32_t view_count;
} FrameBuffer;

event_t display_hw_config();
void display_hw_init(void);
FrameBuffer *begin_render();
void end_render(FrameBuffer *fb);
void end_render_partial(FrameBuffer *fb);
FrameBuffer *peek_framebuffer();
