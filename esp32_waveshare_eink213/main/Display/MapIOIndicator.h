#pragma once

#include "Display/Common.h"
#include "Display/Font.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class MapIOIndicator {
  protected:
    const char *name;
    uint8_t progress;

    FontMedium name_font;

  public:
    explicit MapIOIndicator(const MapIO io_adr);
    MapIOIndicator(const MapIOIndicator &) = delete;
    MapIOIndicator &operator=(const MapIOIndicator &) = delete;
    virtual ~MapIOIndicator();

    void Render(FrameBuffer *fb, Point *start_point, uint8_t progress);
    uint8_t GetHeight();
};
