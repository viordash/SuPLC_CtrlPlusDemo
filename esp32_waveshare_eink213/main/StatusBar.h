#pragma once

#include "DeviceIO.h"
#include "Display/Common.h"
#include "Display/MapIOIndicator.h"
#include <memory>
#include <stdint.h>
#include <unistd.h>
#include <unordered_map>

class StatusBar {
  protected:
    uint8_t y;

    std::unordered_map<MapIO, std::pair<std::unique_ptr<MapIOIndicator>, ControllerBaseInput *>>
        indicators;

    void RenderIndicator(
        FrameBuffer *fb,
        Point *point,
        const std::pair<std::unique_ptr<MapIOIndicator>, ControllerBaseInput *> &indicator);

  public:
    explicit StatusBar(uint8_t y);
    StatusBar(const StatusBar &) = delete;
    virtual ~StatusBar();

    StatusBar &operator=(const StatusBar &) = delete;

    void Render(FrameBuffer *fb);
};
