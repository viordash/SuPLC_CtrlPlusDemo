#include "StatusBar.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatusBar::StatusBar(uint8_t y) {
    this->y = y;

    for (const auto &item : DeviceIO::DigitalInputs) {
        indicators.emplace(std::make_pair(
            item.first,
            std::make_pair(std::make_unique<MapIOIndicator>(item.first), item.second)));
    }
    for (const auto &item : DeviceIO::DigitalOutputs) {
        indicators.emplace(std::make_pair(
            item.first,
            std::make_pair(std::make_unique<MapIOIndicator>(item.first), item.second)));
    }
    for (const auto &item : DeviceIO::AnalogInputs) {
        indicators.emplace(std::make_pair(
            item.first,
            std::make_pair(std::make_unique<MapIOIndicator>(item.first), item.second)));
    }
    for (const auto &item : DeviceIO::Variables) {
        indicators.emplace(std::make_pair(
            item.first,
            std::make_pair(std::make_unique<MapIOIndicator>(item.first), item.second)));
    }
}

StatusBar::~StatusBar() {
}

IRAM_ATTR void StatusBar::RenderIndicator(
    FrameBuffer *fb,
    Point *point,
    const std::pair<std::unique_ptr<MapIOIndicator>, ControllerBaseInput *> &indicator) {
    const uint8_t separator_width = 1;
    indicator.first->Render(fb, point, indicator.second->PeekValue());
    point->x += separator_width;
}

IRAM_ATTR void StatusBar::Render(FrameBuffer *fb) {
    Point point = { 2, y };
    RenderIndicator(fb, &point, indicators[MapIO::A0]);
    RenderIndicator(fb, &point, indicators[MapIO::A1]);
    RenderIndicator(fb, &point, indicators[MapIO::A2]);

    RenderIndicator(fb, &point, indicators[MapIO::D0]);
    RenderIndicator(fb, &point, indicators[MapIO::D1]);
    RenderIndicator(fb, &point, indicators[MapIO::D2]);
    RenderIndicator(fb, &point, indicators[MapIO::D3]);

    RenderIndicator(fb, &point, indicators[MapIO::O0]);
    RenderIndicator(fb, &point, indicators[MapIO::O1]);
    RenderIndicator(fb, &point, indicators[MapIO::O2]);
    RenderIndicator(fb, &point, indicators[MapIO::O3]);

    RenderIndicator(fb, &point, indicators[MapIO::V1]);
    RenderIndicator(fb, &point, indicators[MapIO::V2]);
    RenderIndicator(fb, &point, indicators[MapIO::V3]);
    RenderIndicator(fb, &point, indicators[MapIO::V4]);

    ASSERT(draw_horz_line(fb, 0, y + indicators.begin()->second.first->GetHeight(), DISPLAY_WIDTH));
}