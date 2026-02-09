#include "LogicProgram/Inputs/Indicator.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Indicator = "Indicator";

Indicator::Indicator() : LogicElement(), InputElement(), LabeledElement() {
    high_scale = 100.0f;
    low_scale = 0.0f;
    decimal_point = 2;
    UpdateScale();
    PrintOutValue(0);
}

Indicator::Indicator(const MapIO io_adr, float high_scale, float low_scale, uint8_t decimal_point)
    : LogicElement(), InputElement(), LabeledElement() {
    SetHighScale(high_scale);
    SetLowScale(low_scale);
    SetDecimalPoint(decimal_point);
    SetIoAdr(io_adr);
    UpdateScale();
    PrintOutValue(0);
}

Indicator::~Indicator() {
}

void Indicator::UpdateScale() {
    sprintf(str_format, "%%%u.%uf", max_symbols_count, decimal_point);
}

void Indicator::PrintOutValue(uint8_t eng_value) {
    int32_t eng_range = LogicElement::MaxValue - LogicElement::MinValue;
    float real_range = high_scale - low_scale;
    float ratio = real_range / eng_range;
    float scaled_val = low_scale + (eng_value * ratio);
    snprintf(str_value, sizeof(str_value), str_format, scaled_val);
}

void Indicator::PrintLowScale() {
    sprintf(str_format, "%%0%u.%uf", max_symbols_count, decimal_point);
    snprintf(str_value, sizeof(str_value), str_format, low_scale);
}

void Indicator::AcceptLowScale() {
    low_scale = atof(str_value);
    decimal_point = GetDecimalPointFromScale();
}

void Indicator::PrintHighScale() {
    sprintf(str_format, "%%0%u.%uf", max_symbols_count, decimal_point);
    snprintf(str_value, sizeof(str_value), str_format, high_scale);
}

void Indicator::AcceptHighScale() {
    high_scale = atof(str_value);
    decimal_point = GetDecimalPointFromScale();
}

uint8_t Indicator::GetDecimalPointFromScale() {
    uint8_t point = 1;
    char ch;
    while ((ch = str_value[point]) != 0 && ch != '.' && point < max_symbols_count - 1) {
        point++;
    }
    return max_symbols_count - 1 - point;
}

void Indicator::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

bool Indicator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;
    state = prev_elem_state;

    if (prev_elem_state == LogicItemState::lisActive) {
        if (Controller::RequestWakeupMs(this,
                                        update_period_ms,
                                        ProcessWakeupRequestPriority::pwrp_Idle)
            || prev_elem_changed) {
            any_changes = true;
            switch (editing_property_id) {
                case Indicator::EditingPropertyId::ciepi_None:
                    PrintOutValue(Input->ReadValue());
                    break;
                default:
                    break;
            }
        }
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_Indicator, ".");
    }
    return any_changes;
}

IRAM_ATTR void
Indicator::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, IndicatorLeftPadding));
    } else {
        ASSERT(
            draw_passive_network(fb, start_point->x, start_point->y, IndicatorLeftPadding, false));
    }

    start_point->x += IndicatorLeftPadding;

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (IndicatorHeight / 2)) };
    Point top_right = { (uint16_t)(start_point->x + IndicatorWidth), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (Indicator::EditingPropertyId)editing_property_id
                                     == Indicator::EditingPropertyId::ciepi_None
                              && Blinking_50(fb);
    if (!blink_body_on_editing) {
        ASSERT(draw_rectangle(fb,
                              top_left.x,
                              top_left.y,
                              IndicatorWidth + 1,
                              IndicatorHeight + 1,
                              state != LogicItemState::lisActive));
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (Indicator::EditingPropertyId)editing_property_id
                                      == Indicator::EditingPropertyId::ciepi_ConfigureIOAdr
                               && Blinking_50(fb);
    start_point->x += label_font.GetRightLeftMargin();
    if (!blink_label_on_editing) {
        ASSERT(label_font.DrawText(fb,
                                   start_point->x,
                                   start_point->y - label_font.GetHeight() / 2,
                                   label)
               > 0);
    }
    start_point->x += label_font.GetRightLeftMargin();
    start_point->x += label_width;

    if (!blink_body_on_editing) {
        ASSERT(draw_vert_line(fb,
                              start_point->x,
                              top_left.y,
                              IndicatorHeight,
                              state != LogicItemState::lisActive));
    }
    start_point->x += 1;

    bool show_scales = editable_state == EditableElement::ElementState::des_Editing
                    && (Indicator::EditingPropertyId)editing_property_id
                           != Indicator::EditingPropertyId::ciepi_None
                    && (Indicator::EditingPropertyId)editing_property_id
                           != Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;

    if (show_scales) {
        RenderScales(fb, start_point->x, top_right.x, top_left.y);
    } else {
        ASSERT(value_font.DrawText(fb,
                                   start_point->x + value_font.GetRightLeftMargin(),
                                   top_right.x - value_font.GetRightLeftMargin(),
                                   start_point->y - label_font.GetHeight() / 2,
                                   str_value,
                                   FontBase::Align::Right)
               > 0);
    }

    start_point->x = top_right.x;
    EditableElement::Render(fb, start_point);
}

void Indicator::RenderScales(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    char blink_str_value[sizeof(str_value) + 6];

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            y += 9;
            snprintf(blink_str_value, sizeof(blink_str_value), "%s lo", str_value);
            break;

        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            y += 2;
            snprintf(blink_str_value, sizeof(blink_str_value), "%s hi", str_value);
            break;
    }

    if (Blinking_50(fb)) {
        switch (editing_property_id) {
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
                blink_str_value[0] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
                blink_str_value[1] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
                blink_str_value[2] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
                blink_str_value[3] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
                blink_str_value[4] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
                blink_str_value[5] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
                blink_str_value[6] = ' ';
                break;
            case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
                blink_str_value[7] = ' ';
                break;
        }
    }

    ASSERT(scale_font.DrawText(fb,
                               left + scale_font.GetRightLeftMargin(),
                               right - scale_font.GetRightLeftMargin(),
                               y,
                               blink_str_value,
                               FontBase::Align::Right)
           > 0);
}

size_t Indicator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    auto io_adr = GetIoAdr();
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&low_scale, sizeof(low_scale), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&high_scale, sizeof(high_scale), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&decimal_point, sizeof(decimal_point), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t Indicator::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;
    float _low_scale;
    float _high_scale;
    uint8_t _decimal_point;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    if (!Record::Read(&_low_scale, sizeof(_low_scale), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_low_scale < MinScale || _low_scale > MaxScale) {
        return 0;
    }
    if (!Record::Read(&_high_scale, sizeof(_high_scale), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_high_scale < MinScale || _high_scale > MaxScale) {
        return 0;
    }
    if (!Record::Read(&_decimal_point, sizeof(_decimal_point), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_decimal_point >= max_symbols_count - 1) {
        return 0;
    }
    SetIoAdr(_io_adr);
    SetHighScale(_high_scale);
    SetLowScale(_low_scale);
    decimal_point = _decimal_point;
    UpdateScale();
    return readed;
}

TvElementType Indicator::GetElementType() {
    return TvElementType::et_Indicator;
}

Indicator *Indicator::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Indicator:
            return static_cast<Indicator *>(logic_element);

        default:
            return NULL;
    }
}

void Indicator::SelectPriorSymbol(char *symbol, char extra) {
    switch (*symbol) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *symbol = *symbol - 1;
            break;
        case '0':
            *symbol = extra;
            break;
        default:
            *symbol = '9';
            break;
    }
}

void Indicator::SelectNextSymbol(char *symbol, char extra) {
    switch (*symbol) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            *symbol = *symbol + 1;
            break;
        case '9':
            *symbol = extra;
            break;
        default:
            *symbol = '0';
            break;
    }
}

void Indicator::SelectPrior() {
    log_i(TAG_Indicator, "SelectPrior");

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            SelectPriorSymbol(&str_value[0], '-');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            SelectPriorSymbol(&str_value[1], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            SelectPriorSymbol(&str_value[2], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            SelectPriorSymbol(&str_value[3], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            SelectPriorSymbol(&str_value[4], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            SelectPriorSymbol(&str_value[5], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            SelectPriorSymbol(&str_value[6], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            SelectPriorSymbol(&str_value[7], '9');
            break;
    }
}

void Indicator::SelectNext() {
    log_i(TAG_Indicator, "SelectNext");

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            SelectNextSymbol(&str_value[0], '-');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            SelectNextSymbol(&str_value[1], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            SelectNextSymbol(&str_value[2], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            SelectNextSymbol(&str_value[3], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            SelectNextSymbol(&str_value[4], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            SelectNextSymbol(&str_value[5], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            SelectNextSymbol(&str_value[6], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            SelectNextSymbol(&str_value[7], '0');
            break;
    }
}

void Indicator::PageUp() {
    this->SelectPrior();
}

void Indicator::PageDown() {
    this->SelectNext();
}

void Indicator::Change() {
    log_i(TAG_Indicator, "Change editing_property_id:%d", editing_property_id);
    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_None:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0;
            PrintLowScale();
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0;
            AcceptLowScale();
            PrintHighScale();
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_None;
            AcceptHighScale();
            UpdateScale();
            EndEditing();
            break;
    }
}

void Indicator::Option() {
    log_i(TAG_Indicator, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_None:
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0;
            AcceptLowScale();
            PrintHighScale();
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_None;
            AcceptHighScale();
            UpdateScale();
            EndEditing();
            break;
    }
}

float Indicator::GetLowScale() {
    return low_scale;
}
void Indicator::SetLowScale(float scale) {
    if (scale < -9999999.0f) {
        scale = -9999999.0f;
    }
    if (scale >= 99999990.0f) {
        scale = 99999990.0f;
    }
    low_scale = scale;
}

float Indicator::GetHighScale() {
    return high_scale;
}
void Indicator::SetHighScale(float scale) {
    if (scale < MinScale) {
        scale = MinScale;
    }
    if (scale >= MaxScale) {
        scale = MaxScale;
    }
    high_scale = scale;
}

uint8_t Indicator::GetDecimalPoint() {
    return decimal_point;
}
void Indicator::SetDecimalPoint(uint8_t point) {
    decimal_point = point;
}