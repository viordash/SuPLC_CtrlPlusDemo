
#include "Display/display.h"
#include "Display/fonts/fonts.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_display = "display";

void display_init() {
    if (drawing_init() != 0) {
        log_w(TAG_display, "drawing init failed");
        return;
    }
    display_hw_init();
    log_i(TAG_display, "init succesfully");
}

IRAM_ATTR int draw_char(FrameBuffer *fb,
                        const font_info_t *font,
                        uint16_t x,
                        uint16_t y,
                        char c,
                        pixel_color_t foreground,
                        pixel_color_t background) {
    uint8_t i, j;
    const uint8_t *bitmap;
    uint8_t line = 0;
    int err = 0;

    if (font == NULL) {
        return 0;
    }

    const font_char_desc_t *d = font_get_char_desc(font, c);
    if (d == NULL) {
        return 0;
    }

    bitmap = font->bitmap + d->offset;
    for (j = 0; j < font->height; ++j) {
        for (i = 0; i < d->width; ++i) {
            if (i % 8 == 0) {
                line = bitmap[(d->width + 7) / 8 * j + i / 8]; // line data
            }
            if (line & 0x80) {
                err = drawing_draw_pixel(fb, x + i, y + j, foreground);
            } else {
                err = drawing_draw_pixel(fb, x + i, y + j, background);
            }
            if (err) {
                return -ERANGE;
            }
            line = line << 1;
        }
    }
    return d->width;
}

IRAM_ATTR int draw_string(FrameBuffer *fb,
                          const font_info_t *font,
                          uint16_t x,
                          uint16_t y,
                          const char *str,
                          pixel_color_t foreground,
                          pixel_color_t background) {
    uint16_t t = x;
    int err;

    if (font == NULL || str == NULL)
        return 0;

    while (*str) {
        if ((err = draw_char(fb, font, x, y, *str, foreground, background)) < 0) {
            return err;
        }
        x += err;
        ++str;
        if (*str) {
            x += font->c;
        }
    }
    return x - t;
}

IRAM_ATTR bool draw_active_network(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w) {
    if (w == 0) {
        return true;
    }
    int err = drawing_draw_hline(fb, x, y, w, COLOR_BLACK);
    if (err != 0) {
        return false;
    }
    err = drawing_draw_hline(fb, x, y + 1, w, COLOR_BLACK);
    return err == 0;
}

IRAM_ATTR bool
draw_passive_network(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, bool inverse_dash) {
    (void)inverse_dash;
    int err = drawing_draw_hline(fb, x, y, w, COLOR_GRAY);
    if (err != 0) {
        return false;
    }
    err = drawing_draw_hline(fb, x + 1, y + 1, w, COLOR_GRAY);
    return err == 0;
}

IRAM_ATTR bool draw_active_income_rail(FrameBuffer *fb, uint16_t x, uint16_t y) {
    static_assert(RailsWidth == 2, "RailsWidth == 2");
    int err;
    y -= NETWORK_TOP;
    err = drawing_draw_vline(fb, x, y, RailsHeight, COLOR_BLACK);
    if (err != 0) {
        return false;
    }
    err = drawing_draw_vline(fb, x + 1, y, RailsHeight, COLOR_BLACK);
    return err == 0;
}

IRAM_ATTR bool draw_passive_income_rail(FrameBuffer *fb, uint16_t x, uint16_t y) {
    static_assert(RailsWidth == 2, "RailsWidth == 2");
    int err;
    y -= NETWORK_TOP;
    err = drawing_draw_vline(fb, x, y, RailsHeight, COLOR_BLACK);
    if (err != 0) {
        return false;
    }
    uint16_t height = RailsHeight / 3;
    uint16_t gray_height = RailsHeight - height * 2;
    err = drawing_draw_vline(fb, x + 1, y, height, COLOR_BLACK);
    if (err != 0) {
        return false;
    }
    y += height;
    err = drawing_draw_vline(fb, x + 1, y, gray_height, COLOR_GRAY);
    if (err != 0) {
        return false;
    }
    y += gray_height;
    err = drawing_draw_vline(fb, x + 1, y, height, COLOR_BLACK);
    return err == 0;
}

IRAM_ATTR bool draw_outcome_rail(FrameBuffer *fb, uint16_t x, uint16_t y) {
    static_assert(RailsWidth == 2, "RailsWidth == 2");
    int err;
    y -= NETWORK_TOP;
    uint16_t height = RailsHeight;
    err = drawing_draw_vline(fb, x, y, height, COLOR_BLACK);
    if (err == 0) {
        err = drawing_draw_vline(fb, x + 1, y, height, COLOR_BLACK);
    }
    return err == 0;
}

IRAM_ATTR bool draw_horz_progress_bar(FrameBuffer *fb, uint16_t x, uint16_t y, uint8_t percent04) {
    int width = (HORZ_PROGRESS_BAR_WIDTH * percent04) / 250;
    if (width == 0) {
        return true;
    }
    int err = drawing_draw_hline(fb, x, y + 0, width, COLOR_BLACK);
    if (err != 0) {
        return false;
    }
    err = drawing_draw_hline(fb, x, y + 1, width, COLOR_BLACK);
    return err == 0;
}

IRAM_ATTR bool draw_vert_line(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t h, bool passive) {
    return drawing_draw_vline(fb, x, y, h, !passive ? COLOR_BLACK : COLOR_GRAY) == 0;
}

IRAM_ATTR bool draw_horz_line(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, bool passive) {
    return drawing_draw_hline(fb, x, y, w, !passive ? COLOR_BLACK : COLOR_GRAY) == 0;
}

IRAM_ATTR bool
draw_rectangle(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool passive) {
    return drawing_draw_rectangle(fb, x, y, w, h, !passive ? COLOR_BLACK : COLOR_GRAY) == 0;
}

IRAM_ATTR bool
draw_fill_rectangle(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool passive) {
    return drawing_fill_rectangle(fb, x, y, w, h, !passive ? COLOR_BLACK : COLOR_GRAY) == 0;
}

IRAM_ATTR void draw_bitmap(FrameBuffer *fb, uint16_t x, uint16_t y, const struct Bitmap *bitmap) {
    for (int row = y; row < y + bitmap->size.height; row += 8) {
        if (row >= DISPLAY_HEIGHT) {
            continue;
        }
        for (int column = x; column < x + bitmap->size.width; column++) {
            if (column >= DISPLAY_WIDTH) {
                continue;
            }
            int src_id = (((row - y) / 8) * bitmap->size.width) + (column - x);
            uint8_t b = bitmap->data[src_id];
            for (size_t i = 0; i < 8; i++) {
                if ((b & 1 << i) != 0) {
                    drawing_draw_pixel(fb, column, row + i, COLOR_BLACK);
                }
            }
        }
    }
}
