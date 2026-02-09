#include "LogicProgram/Bindings/DateTimeBinding.h"
#include "DeviceIO.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "icons/datetime_binding.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DateTimeBinding = "DateTimeBinding";

DateTimeBinding::DateTimeBinding() : LogicElement(), InputElement() {
    datetime_part = DatetimePart::t_second;
}

DateTimeBinding::DateTimeBinding(const MapIO io_adr) : DateTimeBinding() {
    SetIoAdr(io_adr);
}

DateTimeBinding::~DateTimeBinding() {
}

void DateTimeBinding::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

bool DateTimeBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        DeviceIO::BindVariableToToDateTime(GetIoAdr(), datetime_part);

    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        DeviceIO::UnbindVariable(GetIoAdr());
    }

    if (state == LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);

        uint32_t event_period_ms;
        switch (datetime_part) {
            case DatetimePart::t_second:
                event_period_ms = 1 * 1000;
                break;
            case DatetimePart::t_minute:
                event_period_ms = 60 * 1 * 1000;
                break;
            case DatetimePart::t_hour:
                event_period_ms = 60 * 60 * 1 * 1000;
                break;
            default:
                event_period_ms = 24 * 60 * 60 * 1 * 1000;
                break;
        }

        log_d(TAG_DateTimeBinding, "%p event_period_ms:%u", this, (unsigned int)event_period_ms);
        Controller::RequestWakeupMs(this,
                                    event_period_ms,
                                    ProcessWakeupRequestPriority::pwrp_Critical);
    } else {
        Controller::RemoveRequestWakeupMs(this);
        log_d(TAG_DateTimeBinding, "%p ", this);
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_DateTimeBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR void
DateTimeBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, DatetimeBindingLeftPadding));
    } else {
        ASSERT(draw_passive_network(fb,
                                    start_point->x,
                                    start_point->y,
                                    DatetimeBindingLeftPadding,
                                    false));
    }

    start_point->x += DatetimeBindingLeftPadding;

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (DatetimeBindingHeight / 2)) };
    Point top_right = { (uint16_t)(start_point->x + DatetimeBindingWidth), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (DateTimeBinding::EditingPropertyId)editing_property_id
                                     == DateTimeBinding::EditingPropertyId::cwbepi_None
                              && Blinking_50(fb);
    if (!blink_body_on_editing) {
        ASSERT(draw_rectangle(fb,
                              top_left.x,
                              top_left.y,
                              DatetimeBindingWidth + 1,
                              DatetimeBindingHeight + 1,
                              state != LogicItemState::lisActive));
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (DateTimeBinding::EditingPropertyId)editing_property_id
                                      == DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr
                               && Blinking_50(fb);
    start_point->x += label_font.GetRightLeftMargin();
    if (!blink_label_on_editing) {
        ASSERT(label_font.DrawText(fb,
                                   start_point->x,
                                   start_point->y - label_font.GetHeight() / 2,
                                   label)
               > 0);
    }
    start_point->x += label_width;
    start_point->x += label_font.GetRightLeftMargin();

    if (!blink_body_on_editing) {
        draw_bitmap(fb,
                    start_point->x,
                    start_point->y - (GetCurrentBitmap()->size.height / 2),
                    GetCurrentBitmap());
    }
    start_point->x += GetCurrentBitmap()->size.width;

    start_point->x += datetime_font.GetRightLeftMargin();
    switch (editing_property_id) {
        case DateTimeBinding::EditingPropertyId::cwbepi_None:
        case DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr:
            ASSERT(datetime_font.DrawText(fb,
                                          start_point->x,
                                          start_point->y - datetime_font.GetHeight() / 2,
                                          GetDatetimePartName())
                   > 0);
            break;

        case DateTimeBinding::EditingPropertyId::cwbepi_SelectDatetimePart:
            if (!Blinking_50(fb)) {
                ASSERT(datetime_font.DrawText(fb,
                                              start_point->x,
                                              start_point->y - datetime_font.GetHeight() / 2,
                                              GetDatetimePartName())
                       > 0);
            }
            break;

        default:
            break;
    }

    start_point->x = top_right.x;
    EditableElement::Render(fb, start_point);
}

const char *DateTimeBinding::GetDatetimePartName() {
    switch (datetime_part) {
        case DatetimePart::t_second:
            return "SECONDS";
        case DatetimePart::t_minute:
            return "MINUTES";
        case DatetimePart::t_hour:
            return "HOURS";
        case DatetimePart::t_day:
            return "DAYS";
        case DatetimePart::t_weekday:
            return "WEEKDAYS";
        case DatetimePart::t_month:
            return "MONTHS";
        case DatetimePart::t_year:
            return "YEARS";
    }
    return NULL;
}

size_t DateTimeBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
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
    if (!Record::Write(&datetime_part, sizeof(datetime_part), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t DateTimeBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;

    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }

    DatetimePart _datetime_part;
    if (!Record::Read(&_datetime_part, sizeof(_datetime_part), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateDatetimePart(_datetime_part)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    datetime_part = _datetime_part;
    return readed;
}

bool DateTimeBinding::ValidateDatetimePart(DatetimePart datetime_part) {
    switch (datetime_part) {
        case t_second:
        case t_minute:
        case t_hour:
        case t_day:
        case t_weekday:
        case t_month:
        case t_year:
            return true;
        default:
            return false;
    }
}

TvElementType DateTimeBinding::GetElementType() {
    return TvElementType::et_DateTimeBinding;
}

DateTimeBinding *DateTimeBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_DateTimeBinding:
            return static_cast<DateTimeBinding *>(logic_element);

        default:
            return NULL;
    }
}

void DateTimeBinding::SelectPrior() {
    log_i(TAG_DateTimeBinding, "SelectPrior");

    switch (editing_property_id) {
        case DateTimeBinding::EditingPropertyId::cwbepi_None:
            break;
        case DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }
        case DateTimeBinding::EditingPropertyId::cwbepi_SelectDatetimePart: {
            auto _datetime_part = (DatetimePart)(datetime_part - 1);
            if (!ValidateDatetimePart(_datetime_part)) {
                _datetime_part = DatetimePart::t_year;
            }
            datetime_part = _datetime_part;
            break;
        }

        default:
            break;
    }
}

void DateTimeBinding::SelectNext() {
    log_i(TAG_DateTimeBinding, "SelectNext");

    switch (editing_property_id) {
        case DateTimeBinding::EditingPropertyId::cwbepi_None:
            break;
        case DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }
        case DateTimeBinding::EditingPropertyId::cwbepi_SelectDatetimePart: {
            auto _datetime_part = (DatetimePart)(datetime_part + 1);
            if (!ValidateDatetimePart(_datetime_part)) {
                _datetime_part = DatetimePart::t_second;
            }
            datetime_part = _datetime_part;
            break;
        }

        default:
            break;
    }
}

void DateTimeBinding::PageUp() {
}

void DateTimeBinding::PageDown() {
}

void DateTimeBinding::Change() {
    switch (editing_property_id) {
        case DateTimeBinding::EditingPropertyId::cwbepi_None:
            editing_property_id = DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr;
            break;
        case DateTimeBinding::EditingPropertyId::cwbepi_ConfigureIOAdr:
            editing_property_id = DateTimeBinding::EditingPropertyId::cwbepi_SelectDatetimePart;
            break;

        default:
            editing_property_id = DateTimeBinding::EditingPropertyId::cwbepi_None;
            EndEditing();
            break;
    }
}
void DateTimeBinding::Option() {
}

const Bitmap *DateTimeBinding::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}