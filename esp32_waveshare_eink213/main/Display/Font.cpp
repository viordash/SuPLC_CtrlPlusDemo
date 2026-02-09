#include "Display/Font.h"
#include "Display/display.h"
#include "Display/fonts/fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FontSmall::FontSmall() : FontBase(font_builtin_fonts[FONT_FACE_SMALL]) {
}

void FontSmall::DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    y += font->height - 1;
    left += font->c + 1;
    right -= 1;
    while (right - left > 0) {
        drawing_draw_pixel(fb, left, y, COLOR_BLACK);
        left += 2;
    }
}

FontMedium::FontMedium() : FontBase(font_builtin_fonts[FONT_FACE_MEDIUM]) {
}

void FontMedium::DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    y += font->height - 2;
    left += font->c + 1;
    right -= 1;
    while (right - left > 0) {
        drawing_draw_pixel(fb, left, y, COLOR_BLACK);
        left += 2;
    }
}

FontLarge::FontLarge() : FontBase(font_builtin_fonts[FONT_FACE_LARGE]) {
}

void FontLarge::DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    y += font->height - 3;
    left += font->c + 1;
    right -= 1;
    while (right - left > 0) {
        drawing_draw_pixel(fb, left, y, COLOR_BLACK);
        left += 2;
    }
}

FontXLarge::FontXLarge() : FontBase(font_builtin_fonts[FONT_FACE_XLARGE]) {
}

void FontXLarge::DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    y += font->height - 4;
    left += font->c + 2;
    right -= 2;
    while (right - left > 0) {
        drawing_draw_pixel(fb, left, y, COLOR_BLACK);
        drawing_draw_pixel(fb, left, y + 1, COLOR_BLACK);
        left += 2;
    }
}

FontBase::FontBase(const font_info_t *font) {
    this->font = font;
}
uint16_t FontBase::GetWidth() {
    return font->char_descriptors[0].width;
}
uint16_t FontBase::MeasureWidth(const char *text) {
    return font_measure_string(font, text);
}
uint16_t FontBase::GetHeight() {
    return font->height;
}
uint16_t FontBase::GetRightLeftMargin() {
    return font->char_descriptors[0].width / 3;
}
uint16_t FontBase::GetTopMargin() {
    return font->height / 3;
}
uint16_t FontBase::GetBottomMargin() {
    return font->avg_bottom_bound;
}

int FontBase::DrawText(FrameBuffer *fb, uint16_t x, uint16_t y, const char *text, bool inverse) {
    return FontBase::DrawText(fb, x, UINT16_MAX, y, text, Align::Left, inverse);
}
int FontBase::DrawText(FrameBuffer *fb, uint16_t x, uint16_t y, const char *text, Align align) {
    return FontBase::DrawText(fb, x, UINT16_MAX, y, text, align);
}
int FontBase::DrawText(FrameBuffer *fb,
                       uint16_t left,
                       uint16_t right,
                       uint16_t y,
                       const char *text,
                       Align align,
                       bool inverse) {

    pixel_color_t foreground = inverse ? COLOR_WHITE : COLOR_BLACK;
    pixel_color_t background = inverse ? COLOR_BLACK : COLOR_WHITE;
    uint16_t draw_width = (right - left) + 1;
    uint16_t text_width = MeasureWidth(text);
    if (text_width <= draw_width) {
        switch (align) {
            case Align::Left:
                return draw_string(fb, font, left, y, text, foreground, background);

            case Align::Right:
                return draw_string(fb, font, right - text_width, y, text, foreground, background);

            case Align::Center: {
                uint16_t left_offset = (draw_width - text_width) / 2;
                return draw_string(fb, font, left + left_offset, y, text, foreground, background);
            }
        }
    }
    return DrawTextWithElipsis(fb, left, right, y, text, text_width, foreground, background);
}

int FontBase::DrawTextWithElipsis(FrameBuffer *fb,
                                  uint16_t left,
                                  uint16_t right,
                                  uint16_t y,
                                  const char *text,
                                  uint16_t text_width,
                                  pixel_color_t foreground,
                                  pixel_color_t background) {

    size_t text_len = strlen(text);
    size_t char_avg_width = text_width / text_len;
    uint16_t elipsis_aprox_width = (char_avg_width * 3) / 2;
    const char *left_str = text;
    const char *right_str = &text[text_len - 1];
    uint16_t draw_width = (right - left) + 1;
    uint16_t rest_width = draw_width;

    while (rest_width > elipsis_aprox_width) {
        const font_char_desc_t *left_char_desc = font_get_char_desc(font, *left_str);
        if (left_char_desc == NULL) {
            return 0;
        }
        const font_char_desc_t *right_char_desc = font_get_char_desc(font, *right_str);
        if (right_char_desc == NULL) {
            return 0;
        }

        bool is_first_symb = left_str == text;
        if (!is_first_symb) {
            left += font->c;
        }
        if (draw_char(fb, font, left, y, *left_str, foreground, background) <= 0) {
            return 0;
        }
        left += left_char_desc->width + font->c;
        left_str++;

        rest_width = right - left;
        if (rest_width <= elipsis_aprox_width) {
            break;
        }

        right -= right_char_desc->width + font->c;
        if (draw_char(fb, font, right, y, *right_str, foreground, background) <= 0) {
            return 0;
        }
        right_str--;
        rest_width = right - left;
    }
    DrawElipsis(fb, left, right, y);
    return draw_width;
}

int FontBase::DrawLimitText(FrameBuffer *fb,
                            uint16_t left,
                            uint16_t right,
                            uint16_t y,
                            const char *text,
                            const char *last_char,
                            bool inverse) {

    pixel_color_t foreground = inverse ? COLOR_WHITE : COLOR_BLACK;
    pixel_color_t background = inverse ? COLOR_BLACK : COLOR_WHITE;

    const font_char_desc_t *desc = font_get_char_desc(font, *last_char);
    if (desc == NULL) {
        return 0;
    }
    right -= desc->width + font->c;
    int draw_width = (right - left) + 1;

    const char *str = text;
    for (const char *s = text; *s != 0 && s < last_char; s++) {
        desc = font_get_char_desc(font, *s);
        if (desc == NULL) {
            return 0;
        }
        if (draw_width > 0) {
            draw_width -= desc->width + font->c;
        }
        bool shift_text_to_right = draw_width <= 0;
        if (shift_text_to_right) {
            str++;
        }
    }

    uint16_t x = left;
    while (right >= left && *str != 0) {
        const font_char_desc_t *left_char_desc = font_get_char_desc(font, *str);
        if (left_char_desc == NULL) {
            return 0;
        }

        bool is_first_symb = str == text;
        if (!is_first_symb) {
            left += font->c;
        }
        if (draw_char(fb, font, left, y, *str, foreground, background) <= 0) {
            return 0;
        }
        left += left_char_desc->width + font->c;
        str++;
    }
    return left - x;
}