#pragma once

#include "Display/Common.h"
#include "Display/drawing.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

#define MAX_SIZE(s, m) (((s) < (m)) ? (s) : (m))

#define NETWORK_TOP (RailsHeight / 2)
#define OUTCOME_RAIL_RIGHT (SCROLLBAR_LEFT - (1 + SCROLLBAR_WIDTH))
#define SCROLLBAR_TOP (RailsTop)
#define SCROLLBAR_WIDTH 1
#define SCROLLBAR_HEIGHT (MAX_VIEW_PORT_COUNT * RailsHeight)
#define SCROLLBAR_LEFT (DISPLAY_WIDTH - SCROLLBAR_WIDTH)
#define VERT_PROGRESS_BAR_HEIGHT 10
#define VERT_PROGRESS_BAR_WIDTH 3
#define HORZ_PROGRESS_BAR_HEIGHT 3
#define HORZ_PROGRESS_BAR_WIDTH 10
#define WIRE_BLINK_BODY_WIDTH 8
#define WIRE_STANDART_WIDTH (16 + WIRE_BLINK_BODY_WIDTH)
#define MAX_VIEW_PORT_COUNT ((DISPLAY_HEIGHT - RailsTop) / RailsHeight)

void display_init();

typedef struct _font_info font_info_t;
int draw_char(FrameBuffer *fb,
              const font_info_t *font,
              uint16_t x,
              uint16_t y,
              char c,
              pixel_color_t foreground,
              pixel_color_t background);
int draw_string(FrameBuffer *fb,
                const font_info_t *font,
                uint16_t x,
                uint16_t y,
                const char *str,
                pixel_color_t foreground,
                pixel_color_t background);

bool draw_active_network(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w);
bool draw_passive_network(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, bool inverse_dash);
bool draw_active_income_rail(FrameBuffer *fb, uint16_t x, uint16_t y);
bool draw_passive_income_rail(FrameBuffer *fb, uint16_t x, uint16_t y);
bool draw_outcome_rail(FrameBuffer *fb, uint16_t x, uint16_t y);
bool draw_vert_progress_bar(FrameBuffer *fb, uint16_t x, uint16_t y, uint8_t percent04);
bool draw_horz_progress_bar(FrameBuffer *fb, uint16_t x, uint16_t y, uint8_t percent04);
bool draw_vert_line(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, bool passive = false);
bool draw_horz_line(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, bool passive = false);
bool draw_rectangle(FrameBuffer *fb,
                    uint16_t x,
                    uint16_t y,
                    uint16_t w,
                    uint16_t h,
                    bool passive = false);
bool draw_fill_rectangle(FrameBuffer *fb,
                         uint16_t x,
                         uint16_t y,
                         uint16_t w,
                         uint16_t h,
                         bool passive = false);
void draw_bitmap(FrameBuffer *fb, uint16_t x, uint16_t y, const struct Bitmap *bitmap);
