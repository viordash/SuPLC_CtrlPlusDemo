#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/Controller.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_CommonTimer = "CommonTimer";

CommonTimer::CommonTimer() : LogicElement() {
    this->delay_time_us = 0;
    str_time[0] = 0;
    str_size = 0;
}

CommonTimer::~CommonTimer() {
}

bool CommonTimer::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        return false;
    }
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        Controller::RequestWakeupMs(this,
                                    delay_time_us / 1000LL,
                                    ProcessWakeupRequestPriority::pwrp_Critical);
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state != LogicItemState::lisActive) {
        state = LogicItemState::lisPassive;
    } else if (state != LogicItemState::lisActive) {
        bool timer_completed =
            Controller::RequestWakeupMs(this,
                                        delay_time_us / 1000LL,
                                        ProcessWakeupRequestPriority::pwrp_Critical);
        if (timer_completed) {
            state = LogicItemState::lisActive;
        }
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_CommonTimer, ".");
    }
    return any_changes;
}

IRAM_ATTR void
CommonTimer::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, TimerLeftPadding));
    } else {
        ASSERT(draw_passive_network(fb, start_point->x, start_point->y, TimerLeftPadding, false));
    }

    start_point->x += TimerLeftPadding;

    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;
    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonTimer::EditingPropertyId)editing_property_id
                                       == CommonTimer::EditingPropertyId::ctepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, bitmap_top, bitmap);
    }

    uint16_t bitmap_left = start_point->x;
    uint16_t bitmap_bottom = bitmap_top + bitmap->size.height;
    start_point->x += bitmap->size.width;
    uint16_t bitmap_right = start_point->x;

    bool blink_value_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonTimer::EditingPropertyId)editing_property_id
                                      == CommonTimer::EditingPropertyId::ctepi_ConfigureDelayTime
                               && Blinking_50(fb);

    switch (str_size) {
        case 1:
            if (!blink_value_on_editing) {
                ASSERT(
                    time_font_1.DrawText(
                        fb,
                        bitmap_left + time_font_1.GetRightLeftMargin(),
                        bitmap_right - time_font_1.GetRightLeftMargin(),
                        bitmap_bottom - (time_font_1.GetHeight() - time_font_1.GetBottomMargin()),
                        str_time,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        case 2:
            if (!blink_value_on_editing) {
                ASSERT(
                    time_font_2.DrawText(
                        fb,
                        bitmap_left + time_font_2.GetRightLeftMargin(),
                        bitmap_right - time_font_2.GetRightLeftMargin(),
                        bitmap_bottom - (time_font_2.GetHeight() - time_font_2.GetBottomMargin()),
                        str_time,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        case 3:
            if (!blink_value_on_editing) {
                ASSERT(
                    time_font_3.DrawText(
                        fb,
                        bitmap_left + time_font_3.GetRightLeftMargin(),
                        bitmap_right - time_font_3.GetRightLeftMargin(),
                        bitmap_bottom - (time_font_3.GetHeight() - time_font_3.GetBottomMargin()),
                        str_time,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        case 4:
            if (!blink_value_on_editing) {
                ASSERT(
                    time_font_4.DrawText(
                        fb,
                        bitmap_left + time_font_4.GetRightLeftMargin(),
                        bitmap_right - time_font_4.GetRightLeftMargin(),
                        bitmap_bottom - (time_font_4.GetHeight() - time_font_4.GetBottomMargin()),
                        str_time,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        default:
            if (blink_value_on_editing) {
                ASSERT(
                    time_font_5.DrawText(
                        fb,
                        bitmap_left + time_font_5.GetRightLeftMargin(),
                        bitmap_right - time_font_5.GetRightLeftMargin(),
                        bitmap_bottom - (time_font_5.GetHeight() - time_font_5.GetBottomMargin()),
                        str_time,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
    }

    EditableElement::Render(fb, start_point);

    log_d(TAG_CommonTimer,
          "Render, str_time:%s, str_size:%d, x:%u, y:%u",
          str_time,
          str_size,
          start_point->x,
          start_point->y);
}

CommonTimer *CommonTimer::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_TimerMSecs:
        case TvElementType::et_TimerSecs:
            return static_cast<CommonTimer *>(logic_element);

        default:
            return NULL;
    }
}