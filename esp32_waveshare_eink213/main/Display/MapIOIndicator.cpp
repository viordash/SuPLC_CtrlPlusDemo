#include "Display/MapIOIndicator.h"
#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MapIOIndicator::MapIOIndicator(const MapIO io_adr) {
    this->name = DeviceIO::GetIOName(io_adr);
    progress = LogicElement::MinValue;
}

MapIOIndicator::~MapIOIndicator() {
}

IRAM_ATTR void MapIOIndicator::Render(FrameBuffer *fb, Point *start_point, uint8_t progress) {
    start_point->x += MapIOIndicatorLeftPadding + name_font.GetRightLeftMargin();

    ASSERT(draw_horz_progress_bar(fb, start_point->x, start_point->y, progress));

    int name_width =
        name_font.DrawText(fb, start_point->x, start_point->y + HORZ_PROGRESS_BAR_HEIGHT, name);
    ASSERT(name_width > 0);

    start_point->x += name_width;

    fb->has_changes |= this->progress != progress;
    this->progress = progress;
}

uint8_t MapIOIndicator::GetHeight() {
    return HORZ_PROGRESS_BAR_HEIGHT + name_font.GetHeight() + MapIOIndicatorBottomPadding;
}