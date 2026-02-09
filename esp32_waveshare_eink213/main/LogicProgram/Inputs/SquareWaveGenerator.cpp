#include "LogicProgram/Inputs/SquareWaveGenerator.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "icons/square_wave_generator.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_SquareWaveGenerator = "SquareWaveGenerator";

SquareWaveGenerator::SquareWaveGenerator() : LogicElement() {
    SetPeriod0(1000);
    SetPeriod1(1000);
}

SquareWaveGenerator::SquareWaveGenerator(uint32_t period0_ms, uint32_t period1_ms)
    : SquareWaveGenerator() {
    SetPeriod0(period0_ms);
    SetPeriod1(period1_ms);
}

SquareWaveGenerator::~SquareWaveGenerator() {
}

bool SquareWaveGenerator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        return false;
    }
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        Controller::RequestWakeupMs(this,
                                    period0_us / 1000LL,
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
                                        period1_us / 1000LL,
                                        ProcessWakeupRequestPriority::pwrp_Critical);
        if (timer_completed) {
            state = LogicItemState::lisActive;
        }
    } else {
        bool timer_completed =
            Controller::RequestWakeupMs(this,
                                        period0_us / 1000LL,
                                        ProcessWakeupRequestPriority::pwrp_Critical);
        if (timer_completed) {
            state = LogicItemState::lisPassive;
        }
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_SquareWaveGenerator, ".");
    }
    return any_changes;
}

IRAM_ATTR void
SquareWaveGenerator::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb,
                                   start_point->x,
                                   start_point->y,
                                   SquareWaveGeneratorLeftPadding));
    } else {
        ASSERT(draw_passive_network(fb,
                                    start_point->x,
                                    start_point->y,
                                    SquareWaveGeneratorLeftPadding,
                                    false));
    }

    start_point->x += SquareWaveGeneratorLeftPadding;

    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;
    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (SquareWaveGenerator::EditingPropertyId)editing_property_id
                                       == SquareWaveGenerator::EditingPropertyId::ctepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, bitmap_top, bitmap);
    }

    uint16_t bitmap_left = start_point->x;
    uint16_t bitmap_bottom = bitmap_top + bitmap->size.height;
    start_point->x += bitmap->size.width;
    uint16_t bitmap_right = start_point->x;

    bool blink_period0_on_editing =
        editable_state == EditableElement::ElementState::des_Editing
        && (SquareWaveGenerator::EditingPropertyId)editing_property_id
               == SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0
        && Blinking_50(fb);

    if (!blink_period0_on_editing) {
        ASSERT(period_font.DrawText(fb,
                                    bitmap_left + period_font.GetRightLeftMargin(),
                                    bitmap_right - period_font.GetRightLeftMargin(),
                                    bitmap_top - 0,
                                    str_period0,
                                    FontBase::Align::Center)
               > 0);
    }

    bool blink_period1_on_editing =
        editable_state == EditableElement::ElementState::des_Editing
        && (SquareWaveGenerator::EditingPropertyId)editing_property_id
               == SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1
        && Blinking_50(fb);

    if (!blink_period1_on_editing) {
        ASSERT(period_font.DrawText(fb,
                                    bitmap_left + period_font.GetRightLeftMargin(),
                                    bitmap_right - period_font.GetRightLeftMargin(),
                                    bitmap_bottom
                                        - (period_font.GetHeight() - period_font.GetBottomMargin()),
                                    str_period1,
                                    FontBase::Align::Center)
               > 0);
    }

    EditableElement::Render(fb, start_point);

    log_d(TAG_SquareWaveGenerator,
          "Render, str_period0:%s, str_period0_size:%d, x:%u, y:%u,",
          str_period0,
          str_period0_size,
          start_point->x,
          start_point->y);
}

void SquareWaveGenerator::SetPeriod0(uint32_t period_ms) {
    SetPeriod(period_ms, &period0_us, str_period0, &str_period0_size);
    log_d(TAG_SquareWaveGenerator, "ctor, str_period0:%s", this->str_period0);
}

void SquareWaveGenerator::SetPeriod1(uint32_t period_ms) {
    SetPeriod(period_ms, &period1_us, str_period1, &str_period1_size);
    log_d(TAG_SquareWaveGenerator, "ctor, str_period0:%s", this->str_period0);
}

void SquareWaveGenerator::SetPeriod(uint32_t period_ms,
                                    uint64_t *period_us,
                                    char *str_period,
                                    size_t *str_period_size) {
    period_ms = std::clamp(period_ms,
                           SquareWaveGenerator::min_period_ms,
                           SquareWaveGenerator::max_period_ms);
    *period_us = period_ms * 1000LL;
    *str_period_size = sprintf(str_period, "%u", (unsigned int)period_ms);
}

uint64_t SquareWaveGenerator::GetPeriod0Us() {
    return this->period0_us;
}

uint64_t SquareWaveGenerator::GetPeriod1Us() {
    return this->period1_us;
}

const Bitmap *SquareWaveGenerator::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

size_t SquareWaveGenerator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&period0_us, sizeof(period0_us), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&period1_us, sizeof(period1_us), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t SquareWaveGenerator::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint64_t _period0_us;
    uint64_t _period1_us;
    if (!Record::Read(&_period0_us, sizeof(_period0_us), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!Record::Read(&_period1_us, sizeof(_period1_us), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_period0_us < SquareWaveGenerator::min_period_ms * 1000LL) {
        return 0;
    }
    if (_period0_us > SquareWaveGenerator::max_period_ms * 1000LL) {
        return 0;
    }
    if (_period1_us < SquareWaveGenerator::min_period_ms * 1000LL) {
        return 0;
    }
    if (_period1_us > SquareWaveGenerator::max_period_ms * 1000LL) {
        return 0;
    }
    SetPeriod0(_period0_us / 1000LL);
    SetPeriod1(_period1_us / 1000LL);
    return readed;
}

TvElementType SquareWaveGenerator::GetElementType() {
    return TvElementType::et_SquareWaveGenerator;
}

SquareWaveGenerator *SquareWaveGenerator::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_SquareWaveGenerator:
            return static_cast<SquareWaveGenerator *>(logic_element);
        default:
            return NULL;
    }
}

void SquareWaveGenerator::SelectPrior() {
    log_i(TAG_SquareWaveGenerator, "SelectPrior");

    switch (editing_property_id) {
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0: {
            uint32_t period_ms = GetPeriod0Us() / 1000L;
            SetPeriod0(period_ms + step_ms);
            break;
        }
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1: {
            uint32_t period_ms = GetPeriod1Us() / 1000L;
            SetPeriod1(period_ms + step_ms);
            break;
        }
        default:
            break;
    }
}

void SquareWaveGenerator::SelectNext() {
    log_i(TAG_SquareWaveGenerator, "SelectNext");

    switch (editing_property_id) {
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0: {
            uint32_t period_ms = std::max(static_cast<uint32_t>(GetPeriod0Us() / 1000L),
                                          SquareWaveGenerator::step_ms);
            SetPeriod0(period_ms - SquareWaveGenerator::step_ms);
            break;
        }
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1: {
            uint32_t period_ms = std::max(static_cast<uint32_t>(GetPeriod1Us() / 1000L),
                                          SquareWaveGenerator::step_ms);
            SetPeriod1(period_ms - SquareWaveGenerator::step_ms);
            break;
        }
        default:
            break;
    }
}

void SquareWaveGenerator::PageUp() {
    switch (editing_property_id) {
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0: {
            uint32_t period_ms = GetPeriod0Us() / 1000L;
            SetPeriod0(period_ms + SquareWaveGenerator::faststep_ms);
            break;
        }
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1: {
            uint32_t period_ms = GetPeriod1Us() / 1000L;
            SetPeriod1(period_ms + SquareWaveGenerator::faststep_ms);
            break;
        }
        default:
            break;
    }
}

void SquareWaveGenerator::PageDown() {
    switch (editing_property_id) {
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0: {
            uint32_t period_ms = std::max(static_cast<uint32_t>(GetPeriod0Us() / 1000L),
                                          SquareWaveGenerator::faststep_ms);
            SetPeriod0(period_ms - SquareWaveGenerator::faststep_ms);
            break;
        }
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1: {
            uint32_t period_ms = std::max(static_cast<uint32_t>(GetPeriod1Us() / 1000L),
                                          SquareWaveGenerator::faststep_ms);
            SetPeriod1(period_ms - SquareWaveGenerator::faststep_ms);
            break;
        }
        default:
            break;
    }
}

void SquareWaveGenerator::Change() {
    log_i(TAG_SquareWaveGenerator, "Change");
    switch (editing_property_id) {
        case SquareWaveGenerator::EditingPropertyId::ctepi_None:
            editing_property_id = SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0;
            break;
        case SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod0:
            editing_property_id = SquareWaveGenerator::EditingPropertyId::ctepi_ConfigurePeriod1;
            break;

        default:
            EndEditing();
            break;
    }
}

void SquareWaveGenerator::Option() {
}