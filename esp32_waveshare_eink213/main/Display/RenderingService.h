#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include "os.h"
#include <mutex>
#include <stdint.h>
#include <unistd.h>

typedef void (*OnExternalRendering)(void *context, FrameBuffer *frame_buffer);

class Ladder;
class RenderingService {
  public:
    static const int DO_RENDERING = BIT1;
    static const int DO_DEFERRED_RENDERING = BIT2;

    struct CachedBitmap {
        uint8_t *bitmap;
        uint32_t last_change_time_ms;
        int32_t view_offset;
        uint32_t view_count;
    };

  protected:
    event_t display_events;
    Ladder *ladder;
    std::mutex render_mutex;
    uint32_t last_change_time_ms;

    static void Task(void *param);

  public:
    explicit RenderingService(event_t display_events);
    RenderingService(RenderingService &) = delete;
    ~RenderingService();
    void Start(Ladder *ladder);
    void Do();
    void DoDeferred();

    CachedBitmap BeginRenderOnExternal();
    void EndRenderOnExternal();
};
