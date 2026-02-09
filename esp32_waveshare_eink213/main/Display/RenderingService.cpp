#include "Display/RenderingService.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "StatusBar.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_RenderingService = "RenderingService";

RenderingService::RenderingService(event_t display_events) {
    this->display_events = display_events;
    ladder = nullptr;
    last_change_time_ms = 0;
}

RenderingService::~RenderingService() {
}

void RenderingService::Task(void *parm) {
    log_i(TAG_RenderingService, "start task");
    RenderingService *service = static_cast<RenderingService *>(parm);

    uint32_t full_refresh_counter = 0;
    StatusBar statusBar(0);
    const uint32_t display_render_on_startup = DO_RENDERING;
    uint32_t ulNotifiedValue = display_render_on_startup;

    while (true) {
        log_d(TAG_RenderingService, "ulNotifiedValue:0x%08X", (unsigned int)ulNotifiedValue);
        if (ulNotifiedValue & DO_RENDERING) {
            uint64_t time_before_render = timer_get_time_us();

            std::lock_guard<std::mutex> lock(service->render_mutex);
            {
                FrameBuffer *fb = begin_render();
                statusBar.Render(fb);
                service->ladder->Render(fb);

                const uint32_t full_refresh_timer = 3600;
                bool full_refresh = full_refresh_counter % full_refresh_timer == 0;
                full_refresh_counter++;

                bool force = ulNotifiedValue & DO_DEFERRED_RENDERING;
                fb->has_changes |= force;
                if (full_refresh) {
                    end_render(fb);
                } else {
                    end_render_partial(fb);
                }
                if (fb->has_changes) {
                    service->last_change_time_ms = time_before_render / 1000;
                }
            }

            uint64_t time_after_render = timer_get_time_us();
            static uint64_t loop_time = 0;
            log_d(TAG_RenderingService,
                  "r %d ms (%d ms)",
                  (int)((time_after_render - loop_time) / 1000),
                  (int)((time_after_render - time_before_render) / 1000));
            loop_time = time_after_render;
        }
        wait_event(service->display_events, &ulNotifiedValue);

        if (ulNotifiedValue & DO_DEFERRED_RENDERING) {
            wait_event_timed(service->display_events,
                             &ulNotifiedValue,
                             DISPLAY_MIN_PERIOD_RENDER_MS);
            ulNotifiedValue |= DO_RENDERING | DO_DEFERRED_RENDERING;
        }
    }
    log_i(TAG_RenderingService, "stop task");
    delete_current_task();
}

void RenderingService::Start(Ladder *ladder) {
    this->ladder = ladder;
    task_handle_t task_handle = create_task(Task,
                                            "render_task",
                                            RenderingService_Task_Stack_size,
                                            this,
                                            task_priority_t::tp_Idle);

    subscribe_to_event(display_events, task_handle);
}

void RenderingService::Do() {
    log_d(TAG_RenderingService, "do");
    raise_event(display_events, DO_RENDERING);
}

void RenderingService::DoDeferred() {
    log_d(TAG_RenderingService, "do deffered");
    raise_event(display_events, DO_DEFERRED_RENDERING);
}

RenderingService::CachedBitmap RenderingService::BeginRenderOnExternal() {
    render_mutex.lock();
    FrameBuffer *fb = peek_framebuffer();

    log_d(TAG_RenderingService,
          "BeginRenderOnExternal, last_change_time_ms:%u",
          (unsigned)last_change_time_ms);

    return { fb->buffer, last_change_time_ms, fb->view_offset, fb->view_count };
}

void RenderingService::EndRenderOnExternal() {
    render_mutex.unlock();
}
