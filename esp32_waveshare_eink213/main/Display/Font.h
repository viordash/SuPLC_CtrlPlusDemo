#pragma once

#include "Display/drawing.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

typedef struct _font_info font_info_t;

class FontBase {
  protected:
    const font_info_t *font;

    virtual void DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) = 0;
    int DrawTextWithElipsis(FrameBuffer *fb,
                            uint16_t left,
                            uint16_t right,
                            uint16_t y,
                            const char *text,
                            uint16_t text_width,
                            pixel_color_t foreground,
                            pixel_color_t background);

  public:
    enum Align { Left, Center, Right };
    explicit FontBase(const font_info_t *font_info_t);
    int DrawText(FrameBuffer *fb, uint16_t x, uint16_t y, const char *text, bool inverse);
    int
    DrawText(FrameBuffer *fb, uint16_t x, uint16_t y, const char *text, Align align = Align::Left);
    int DrawText(FrameBuffer *fb,
                 uint16_t left,
                 uint16_t right,
                 uint16_t y,
                 const char *text,
                 Align align,
                 bool inverse = false);

    int DrawLimitText(FrameBuffer *fb,
                      uint16_t left,
                      uint16_t right,
                      uint16_t y,
                      const char *text,
                      const char *last_char,
                      bool inverse = false);

    uint16_t GetWidth();
    uint16_t MeasureWidth(const char *text);
    uint16_t GetHeight();
    uint16_t GetRightLeftMargin();
    uint16_t GetTopMargin();
    uint16_t GetBottomMargin();
};

class FontSmall : public FontBase {
  protected:
    void DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) override;

  public:
    explicit FontSmall();
};

class FontMedium : public FontBase {
  protected:
    void DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) override;

  public:
    explicit FontMedium();
};

class FontLarge : public FontBase {
  protected:
    void DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) override;

  public:
    explicit FontLarge();
};

class FontXLarge : public FontBase {
  protected:
    void DrawElipsis(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) override;

  public:
    explicit FontXLarge();
};