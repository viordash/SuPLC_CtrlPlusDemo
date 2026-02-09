
#include "Display/drawing.h"
#include "Display/display.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define abs(x) ((x) < 0 ? -(x) : (x))
#define swap(x, y)                                                                                 \
    do {                                                                                           \
        typeof(x) temp##x##y = x;                                                                  \
        x = y;                                                                                     \
        y = temp##x##y;                                                                            \
    } while (0)

int drawing_init() {
    return 0;
}

IRAM_ATTR int drawing_draw_pixel(FrameBuffer *fb, uint16_t x, uint16_t y, pixel_color_t color) {
    uint16_t index;

    if (fb == FRAMEBUFFER_STUB) {
        return 0;
    }
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return -EINVAL;
    }

    int scan_line = (DISPLAY_HEIGHT + 7) / 8;
    index = (y / 8) + ((DISPLAY_WIDTH - 1) - x) * scan_line;
    uint8_t mask = 0x80 >> (y & 7);
    // printf("x:%d, y:%d, id:%d, mask:0x%02X, fb[]:0x%02X, color:%u\n", x, y, index, mask, fb->buffer[index], color);

    switch (color) {
        case COLOR_WHITE:
            fb->buffer[index] |= mask;
            break;
        case COLOR_BLACK:
        case COLOR_GRAY:
            fb->buffer[index] &= ~mask;
            break;
        default:
            break;
    }

    return 0;
}

IRAM_ATTR int
drawing_draw_hline(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, pixel_color_t color) {
    uint16_t index;
    uint8_t mask;

    if (fb == FRAMEBUFFER_STUB) {
        return 0;
    }
    // boundary check
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return -EINVAL;
    }
    if (w == 0) {
        return -EINVAL;
    }
    if (x + w > DISPLAY_WIDTH) {
        w = DISPLAY_WIDTH - x;
    }

    int scan_line = (DISPLAY_HEIGHT + 7) / 8;
    index = (y / 8) + ((DISPLAY_WIDTH - 1) - x) * scan_line;
    mask = 0x80 >> (y & 7);
    // printf("x:%d, y:%d, mask:%d, height:%d, width:%d, line:%d, index:%d\n",
    //        x,
    //        y,
    //        mask,
    //        DISPLAY_HEIGHT,
    //        DISPLAY_WIDTH,
    //        scan_line,
    //        index);
    switch (color) {
        case COLOR_WHITE:
            while (w--) {
                fb->buffer[index] |= mask;
                index -= scan_line;
            }
            break;
        case COLOR_BLACK:
            mask = ~mask;
            while (w--) {
                fb->buffer[index] &= mask;
                // printf(". %d, 0x%02X\n", index, mask);
                index -= scan_line;
            }
            break;
        case COLOR_GRAY:
            mask = ~mask;
            while (w--) {
                if (w & 0x01) {
                    fb->buffer[index] &= mask;
                }
                index -= scan_line;
            }
            break;
        default:
            break;
    }
    return 0;
}

IRAM_ATTR int
drawing_draw_vline(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t h, pixel_color_t color) {
    uint16_t index;
    uint8_t mask;

    if (fb == FRAMEBUFFER_STUB) {
        return 0;
    }
    // boundary check
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return -EINVAL;
    }
    if (h == 0) {
        return -EINVAL;
    }
    if (y + h > DISPLAY_HEIGHT) {
        h = DISPLAY_HEIGHT - y;
    }

    int scan_line = (DISPLAY_HEIGHT + 7) / 8;
    index = (y / 8) + ((DISPLAY_WIDTH - 1) - x) * scan_line;

    uint8_t top = y & 7;
    // printf("x:%d, y:%d, height:%d, top:%d, line:%d, index:%d\n", x, y, h, top, scan_line, index);

    if (top) {
        static const uint8_t premask[] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        static const uint8_t premask_g_odd[] = { 0x00, 0xAA, 0xEA, 0xEA, 0xFA, 0xFA, 0xFE, 0xFE };
        static const uint8_t premask_g_even[] = { 0x00, 0xD5, 0xD5, 0xF5, 0xF5, 0xFD, 0xFD, 0xFF };

        mask = premask[top];
        if (h < top) {
            mask &= (0xFF >> (top - h));
        }
        switch (color) {
            case COLOR_WHITE:
                fb->buffer[index] |= ~mask;
                break;
            case COLOR_BLACK:
                fb->buffer[index] &= mask;
                // printf(". %d, 0x%02X\n", index, mask);
                break;
            case COLOR_GRAY:
                if (y & 0x01) {
                    mask = premask_g_odd[top] | mask;
                } else {
                    mask = premask_g_even[top] | mask;
                }
                if (h < top) {
                    mask &= (0xFF >> (top - h));
                }
                fb->buffer[index] &= mask;
                break;
            default:
                break;
        }

        if (h < 8 - top) {
            return 0;
        }
        h -= 8 - top;
        index++;
    }
    // printf("h:%d\n", h);

    if (h >= 8) { // byte aligned line at middle
        switch (color) {
            case COLOR_WHITE:
                do {
                    fb->buffer[index] = 0xff;
                    index++;
                    h -= 8;
                } while (h >= 8);
                break;
            case COLOR_BLACK:
                do {
                    fb->buffer[index] = 0x00;
                    // printf(". %d, 0x%02X\n", index, 0);
                    index++;
                    h -= 8;
                } while (h >= 8);
                break;
            case COLOR_GRAY:
                do {
                    if (y & 0x01) {
                        fb->buffer[index] = 0xaa;
                    } else {
                        fb->buffer[index] = 0x55;
                    }
                    index++;
                    h -= 8;
                } while (h >= 8);
                break;
            default:
                break;
        }
    }
    if (h > 0) {
        uint8_t bottom = h & 7;
        static const uint8_t postmask[] = { 0x00, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };
        static const uint8_t premask_g_odd[] = { 0x00, 0xBF, 0xBF, 0xAF, 0xAF, 0xAB, 0xAB, 0xAA };
        static const uint8_t premask_g_even[] = { 0x00, 0x7F, 0x7F, 0x5F, 0x5F, 0x57, 0x57, 0x55 };
        mask = postmask[bottom];
        switch (color) {
            case COLOR_WHITE:
                fb->buffer[index] |= ~mask;
                break;
            case COLOR_BLACK:
                fb->buffer[index] &= mask;
                // printf(". %d, 0x%02X, bottom:%d\n", index, mask, bottom);
                break;
            case COLOR_GRAY:
                if (y & 0x01) {
                    mask = premask_g_odd[bottom] | mask;
                } else {
                    mask = premask_g_even[bottom] | mask;
                }
                fb->buffer[index] &= mask;
                break;
            default:
                break;
        }
    }
    return 0;
}

int drawing_draw_rectangle(FrameBuffer *fb,
                           uint16_t x,
                           uint16_t y,
                           uint16_t w,
                           uint16_t h,
                           pixel_color_t color) {
    int err = 0;
    if ((err = drawing_draw_hline(fb, x, y, w, color)))
        return err;
    if ((err = drawing_draw_hline(fb, x, y + h - 1, w, color)))
        return err;
    if ((err = drawing_draw_vline(fb, x, y, h, color)))
        return err;
    return drawing_draw_vline(fb, x + w - 1, y, h, color);
}

int drawing_fill_rectangle(FrameBuffer *fb,
                           uint16_t x,
                           uint16_t y,
                           uint16_t w,
                           uint16_t h,
                           pixel_color_t color) {
    // Can be optimized?
    uint8_t i;
    int err;
    for (i = x; i < x + w; ++i)
        if ((err = drawing_draw_vline(fb, i, y, h, color)))
            return err;
    return 0;
}

int drawing_draw_circle(FrameBuffer *fb, uint16_t x0, uint16_t y0, uint8_t r, pixel_color_t color) {
    // Refer to http://en.wikipedia.org/wiki/Midpoint_circle_algorithm for the algorithm
    uint16_t x = r;
    uint16_t y = 1;
    int16_t radius_err = 1 - x;
    int err = 0;

    if (r == 0)
        return -EINVAL;

    if ((err = drawing_draw_pixel(fb, x0 - r, y0, color)))
        return err;
    if ((err = drawing_draw_pixel(fb, x0 + r, y0, color)))
        return err;
    if ((err = drawing_draw_pixel(fb, x0, y0 - r, color)))
        return err;
    if ((err = drawing_draw_pixel(fb, x0, y0 + r, color)))
        return err;

    while (x >= y) {
        if ((err = drawing_draw_pixel(fb, x0 + x, y0 + y, color)))
            return err;
        if ((err = drawing_draw_pixel(fb, x0 - x, y0 + y, color)))
            return err;
        if ((err = drawing_draw_pixel(fb, x0 + x, y0 - y, color)))
            return err;
        if ((err = drawing_draw_pixel(fb, x0 - x, y0 - y, color)))
            return err;
        if (x != y) {
            /* Otherwise the 4 drawings below are the same as above, causing
             * problem when color is INVERT
             */
            if ((err = drawing_draw_pixel(fb, x0 + y, y0 + x, color)))
                return err;
            if ((err = drawing_draw_pixel(fb, x0 - y, y0 + x, color)))
                return err;
            if ((err = drawing_draw_pixel(fb, x0 + y, y0 - x, color)))
                return err;
            if ((err = drawing_draw_pixel(fb, x0 - y, y0 - x, color)))
                return err;
        }
        ++y;
        if (radius_err < 0) {
            radius_err += 2 * y + 1;
        } else {
            --x;
            radius_err += 2 * (y - x + 1);
        }
    }
    return 0;
}

IRAM_ATTR int
drawing_fill_circle(FrameBuffer *fb, uint16_t x0, uint16_t y0, uint8_t r, pixel_color_t color) {
    uint16_t x = 1;
    uint16_t y = r;
    int16_t radius_err = 1 - y;
    int err = 0;

    if (r == 0)
        return -EINVAL;

    if ((err = drawing_draw_vline(fb, x0, y0 - r, 2 * r + 1, color))) // Center vertical line
        return err;
    while (y >= x) {
        if ((err = drawing_draw_vline(fb, x0 - x, y0 - y, 2 * y + 1, color)))
            return err;
        if ((err = drawing_draw_vline(fb, x0 + x, y0 - y, 2 * y + 1, color)))
            return err;
        if ((err = drawing_draw_vline(fb, x0 - y, y0 - x, 2 * x + 1, color)))
            return err;
        if ((err = drawing_draw_vline(fb, x0 + y, y0 - x, 2 * x + 1, color)))
            return err;
        ++x;
        if (radius_err < 0) {
            radius_err += 2 * x + 1;
        } else {
            --y;
            radius_err += 2 * (x - y + 1);
        }
    }
    return 0;
}

int drawing_draw_line(FrameBuffer *fb,
                      int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      pixel_color_t color) {
    if ((x0 >= DISPLAY_WIDTH) || (x0 < 0) || (y0 >= DISPLAY_HEIGHT) || (y0 < 0))
        return -EINVAL;
    if ((x1 >= DISPLAY_WIDTH) || (x1 < 0) || (y1 >= DISPLAY_HEIGHT) || (y1 < 0))
        return -EINVAL;

    int err;
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t errx = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            if ((err = drawing_draw_pixel(fb, y0, x0, color)))
                return err;
        } else {
            if ((err = drawing_draw_pixel(fb, x0, y0, color)))
                return err;
        }
        errx -= dy;
        if (errx < 0) {
            y0 += ystep;
            errx += dx;
        }
    }
    return 0;
}

int drawing_draw_triangle(FrameBuffer *fb,
                          int16_t x0,
                          int16_t y0,
                          int16_t x1,
                          int16_t y1,
                          int16_t x2,
                          int16_t y2,
                          pixel_color_t color) {
    int err;
    if ((err = drawing_draw_line(fb, x0, y0, x1, y1, color)))
        return err;
    if ((err = drawing_draw_line(fb, x1, y1, x2, y2, color)))
        return err;
    return drawing_draw_line(fb, x2, y2, x0, y0, color);
}

int drawing_fill_triangle(FrameBuffer *fb,
                          int16_t x0,
                          int16_t y0,
                          int16_t x1,
                          int16_t y1,
                          int16_t x2,
                          int16_t y2,
                          pixel_color_t color) {
    int16_t a, b, y, last;
    int err;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        swap(y0, y1);
        swap(x0, x1);
    }
    if (y1 > y2) {
        swap(y2, y1);
        swap(x2, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
        swap(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        if ((err = drawing_draw_hline(fb, a, y0, b - a + 1, color)))
            return err;
        return 0;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1,
            dy12 = y2 - y1, sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            swap(a, b);
        if ((err = drawing_draw_hline(fb, a, y, b - a + 1, color)))
            return err;
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            swap(a, b);
        if ((err = drawing_draw_hline(fb, a, y, b - a + 1, color)))
            return err;
    }
    return 0;
}