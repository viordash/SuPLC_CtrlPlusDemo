#pragma once

#include "Display/Font.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

class ListBox {
  protected:
    const static int line_size = 21;
    char title[line_size];
    char lines[LisBoxLinesCount][line_size];

    int selected;
    int title_x;
    bool frame_buffer_req_render;

    FontLarge title_font;

    void BuildTitle(const char *title);

  public:
    explicit ListBox(const char *title);

    void Render(FrameBuffer *fb);
    bool Insert(int pos, const char *text);
    void Select(int index);
};
