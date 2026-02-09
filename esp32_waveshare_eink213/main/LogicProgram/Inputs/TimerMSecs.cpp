#include "LogicProgram/Inputs/TimerMSecs.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/timer_msec.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerMSecs = "TimerMSecs";

TimerMSecs::TimerMSecs() : CommonTimer() {
}

TimerMSecs::TimerMSecs(uint32_t delay_time_ms) : TimerMSecs() {
    SetTime(delay_time_ms);
}

TimerMSecs::~TimerMSecs() {
}

void TimerMSecs::SetTime(uint32_t delay_time_ms) {
    delay_time_ms =
        std::clamp(delay_time_ms, TimerMSecs::min_delay_time_ms, TimerMSecs::max_delay_time_ms);
    this->delay_time_us = delay_time_ms * 1000LL;
    str_size = sprintf(this->str_time, "%u", (unsigned int)delay_time_ms);

    log_d(TAG_TimerMSecs, "ctor, str_time:%s", this->str_time);
}

uint64_t TimerMSecs::GetTimeUs() {
    return this->delay_time_us;
}

const Bitmap *TimerMSecs::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

size_t TimerMSecs::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&delay_time_us, sizeof(delay_time_us), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t TimerMSecs::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint64_t _delay_time_us;
    if (!Record::Read(&_delay_time_us, sizeof(_delay_time_us), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_delay_time_us < TimerMSecs::min_delay_time_ms * 1000LL) {
        return 0;
    }
    if (_delay_time_us > TimerMSecs::max_delay_time_ms * 1000LL) {
        return 0;
    }
    SetTime(_delay_time_us / 1000LL);
    return readed;
}

TvElementType TimerMSecs::GetElementType() {
    return TvElementType::et_TimerMSecs;
}

TimerMSecs *TimerMSecs::TryToCast(CommonTimer *common_timer) {
    switch (common_timer->GetElementType()) {
        case TvElementType::et_TimerMSecs:
            return static_cast<TimerMSecs *>(common_timer);

        default:
            return NULL;
    }
}

void TimerMSecs::SelectPrior() {
    log_i(TAG_TimerMSecs, "SelectPrior");
    uint32_t delay_time_ms = GetTimeUs() / 1000L;
    SetTime(delay_time_ms + TimerMSecs::step_ms);
}

void TimerMSecs::SelectNext() {
    log_i(TAG_TimerMSecs, "SelectNext");
    uint32_t delay_time_ms =
        std::max(static_cast<uint32_t>(GetTimeUs() / 1000L), TimerMSecs::step_ms);
    SetTime(delay_time_ms - TimerMSecs::step_ms);
}

void TimerMSecs::PageUp() {
    uint32_t delay_time_ms = GetTimeUs() / 1000L;
    SetTime(delay_time_ms + TimerMSecs::faststep_ms);
}

void TimerMSecs::PageDown() {
    uint32_t delay_time_ms =
        std::max(static_cast<uint32_t>(GetTimeUs() / 1000L), TimerMSecs::faststep_ms);
    SetTime(delay_time_ms - TimerMSecs::faststep_ms);
}

void TimerMSecs::Change() {
    log_i(TAG_TimerMSecs, "Change");
    switch (editing_property_id) {
        case TimerMSecs::EditingPropertyId::ctepi_None:
            editing_property_id = TimerMSecs::EditingPropertyId::ctepi_ConfigureDelayTime;
            break;

        default:
            EndEditing();
            break;
    }
}

void TimerMSecs::Option() {
}