#include "LogicProgram/Controls/TrackBarControl.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUPPORT_INTERACTIVE

static const char *TAG_TrackBarControl = "TrackBarControl";

TrackBarControl::TrackBarControl()
    : LogicElement(), InteractiveElement(), InputOutputElement(), LabeledElement() {
    SetPointerPosition(0);
    touch_screen_pressed = false;
}

TrackBarControl::TrackBarControl(const MapIO io_adr) : TrackBarControl() {
    SetIoAdr(io_adr);
}

TrackBarControl::~TrackBarControl() {
}

void TrackBarControl::SetIoAdr(const MapIO io_adr) {
    InputOutputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

bool TrackBarControl::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        touch_screen_pressed = false;
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;
    state = prev_elem_state;

    if (prev_elem_state == LogicItemState::lisActive) {
        Point coord;
        bool pressed = get_touch_screen_pressed(&coord);
        bool touch_screen_changed = touch_screen_pressed != pressed;
        touch_screen_pressed = pressed;
        if (touch_screen_changed) {
            const int left_limit = -10;
            const int right_limit = 110;
            int x_line = interactive_area.bottom_right.x - interactive_area.top_left.x;
            int x_pos = coord.x - interactive_area.top_left.x;
            int x_relative_perc = (100 * x_pos) / x_line;
            bool in_y_area = coord.y >= interactive_area.top_left.y
                          && coord.y <= interactive_area.bottom_right.y;
            bool in_x_area = x_relative_perc >= left_limit && x_relative_perc <= right_limit;

            if (in_y_area && in_x_area) {
                int eng_range = LogicElement::MaxValue - LogicElement::MinValue;
                uint8_t eng_value = (std::clamp(x_relative_perc, 0, 100) * eng_range) / 100;
                Output->WriteValue(eng_value);
                SetPointerPosition(eng_value);
                any_changes = true;
                log_i(TAG_TrackBarControl,
                      "Pressed: %u, x_relative_perc: %d, pointer_val: %u, eng_value: %u",
                      pressed,
                      x_relative_perc,
                      pointer_val,
                      eng_value);
            }
        }

        if (!any_changes
            && Controller::RequestWakeupMs(this,
                                           update_period_ms,
                                           ProcessWakeupRequestPriority::pwrp_Idle)) {
            any_changes = true;
            switch (editing_property_id) {
                case TrackBarControl::EditingPropertyId::tbepi_None:
                    SetPointerPosition(Output->ReadValue());
                    break;
                default:
                    break;
            }
        }
    } else {
        touch_screen_pressed = false;
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_TrackBarControl, ".");
    }
    return any_changes;
}

IRAM_ATTR void
TrackBarControl::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, TrackBarControlLeftPadding));
    } else {
        ASSERT(draw_passive_network(fb,
                                    start_point->x,
                                    start_point->y,
                                    TrackBarControlLeftPadding,
                                    false));
    }

    start_point->x += TrackBarControlLeftPadding;

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (TrackBarControlHeight / 2)) };
    Point bottom_left = { start_point->x, (uint16_t)(top_left.y + TrackBarControlHeight) };
    Point top_right = { (uint16_t)(start_point->x + TrackBarControlWidth), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (TrackBarControl::EditingPropertyId)editing_property_id
                                     == TrackBarControl::EditingPropertyId::tbepi_None
                              && Blinking_50(fb);
    if (!blink_body_on_editing) {
        ASSERT(draw_rectangle(fb,
                              top_left.x,
                              top_left.y,
                              TrackBarControlWidth + 1,
                              TrackBarControlHeight + 1,
                              state != LogicItemState::lisActive));
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (TrackBarControl::EditingPropertyId)editing_property_id
                                      == TrackBarControl::EditingPropertyId::tbepi_ConfigureIOAdr
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
                              TrackBarControlHeight,
                              state != LogicItemState::lisActive));
    }
    start_point->x += 1;

    uint16_t scale_width = top_right.x - start_point->x;
    uint16_t ticks_step = scale_width / (TrackBarControlTicksCount + 1);
    uint16_t point_side = std::max(ticks_step / 4, 2);
    uint16_t ticks_width = scale_width - (point_side * 2);
    ticks_step = ticks_width / TrackBarControlTicksCount;
    ticks_width = ticks_step * (TrackBarControlTicksCount - 1);
    scale_width = ticks_width + (point_side * 2);

    uint16_t scale_left = start_point->x + ((top_right.x - start_point->x) - scale_width) / 2;
    uint16_t ticks_indent = point_side;
    uint16_t ticks_left = scale_left + ticks_indent;

    uint16_t point_left = ((uint32_t)ticks_width * (uint32_t)pointer_val) / 100;
    point_left += ticks_left;
    uint16_t point_width = point_side * 2 + 1;

    uint16_t scale_top = top_left.y + TrackBarControlHeight / 3;
    uint16_t ticks_top = scale_top + TrackBarControlHeight / 2;
    uint16_t ticks_height = TrackBarControlHeight / 6;

    uint16_t point_top = top_left.y + (scale_top - top_left.y) / 2;
    uint16_t point_height = ticks_top - point_top - 1;

    Point area_top_left = { scale_left, top_left.y };
    Point area_bottom_right = { (uint16_t)(scale_left + scale_width), bottom_left.y };
    SetInteractiveArea(&area_top_left, &area_bottom_right);

    if (!blink_body_on_editing) {
        ASSERT(draw_horz_line(fb,
                              scale_left,
                              scale_top,
                              scale_width,
                              state != LogicItemState::lisActive));

        for (size_t i = 0; i < TrackBarControlTicksCount; i++) {
            ASSERT(draw_vert_line(fb,
                                  ticks_left,
                                  ticks_top,
                                  ticks_height,
                                  state != LogicItemState::lisActive));
            ticks_left += ticks_step;
        }

        ASSERT(draw_fill_rectangle(fb,
                                   point_left - point_side,
                                   point_top,
                                   point_width,
                                   point_height,
                                   state != LogicItemState::lisActive));
    }

    start_point->x = top_right.x;
    EditableElement::Render(fb, start_point);
}

void TrackBarControl::SetPointerPosition(uint8_t eng_value) {
    int32_t eng_range = LogicElement::MaxValue - LogicElement::MinValue;
    float low_scale = 0.0;
    float high_scale = 100.0;
    float real_range = high_scale - low_scale;
    float ratio = real_range / eng_range;
    float scaled_val = low_scale + (eng_value * ratio);
    pointer_val = scaled_val;
}

size_t TrackBarControl::Serialize(uint8_t *buffer, size_t buffer_size) {
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
    return writed;
}

size_t TrackBarControl::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;

    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    return readed;
}

TvElementType TrackBarControl::GetElementType() {
    return TvElementType::et_TrackBarControl;
}

TrackBarControl *TrackBarControl::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_TrackBarControl:
            return static_cast<TrackBarControl *>(logic_element);

        default:
            return NULL;
    }
}

void TrackBarControl::SelectPrior() {
    log_i(TAG_TrackBarControl, "SelectPrior");
    switch (editing_property_id) {
        case TrackBarControl::EditingPropertyId::tbepi_None:
            break;
        case TrackBarControl::EditingPropertyId::tbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        default:
            break;
    }
}

void TrackBarControl::SelectNext() {
    log_i(TAG_TrackBarControl, "SelectNext");

    switch (editing_property_id) {
        case TrackBarControl::EditingPropertyId::tbepi_None:
            break;
        case TrackBarControl::EditingPropertyId::tbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        default:
            break;
    }
}

void TrackBarControl::PageUp() {
}

void TrackBarControl::PageDown() {
}

void TrackBarControl::Change() {
    log_i(TAG_TrackBarControl, "Change");
    switch (editing_property_id) {
        case TrackBarControl::EditingPropertyId::tbepi_None:
            editing_property_id = TrackBarControl::EditingPropertyId::tbepi_ConfigureIOAdr;
            break;

        default:
            editing_property_id = TrackBarControl::EditingPropertyId::tbepi_None;
            EndEditing();
            break;
    }
}

void TrackBarControl::Option() {
}

#endif //SUPPORT_INTERACTIVE