#include "LogicProgram/Bindings/CommonWiFiBinding.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "icons/wifi_binding.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonWiFiBinding = "CommonWiFiBinding";

CommonWiFiBinding::CommonWiFiBinding() : LogicElement(), InputElement(), LabeledElement() {
}

CommonWiFiBinding::CommonWiFiBinding(const MapIO io_adr) : CommonWiFiBinding() {
    SetIoAdr(io_adr);
}

CommonWiFiBinding::~CommonWiFiBinding() {
}

void CommonWiFiBinding::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

IRAM_ATTR void
CommonWiFiBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, WifiBindingLeftPadding));
    } else {
        ASSERT(draw_passive_network(fb,
                                    start_point->x,
                                    start_point->y,
                                    WifiBindingLeftPadding,
                                    false));
    }

    start_point->x += WifiBindingLeftPadding;

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (WifiBindingHeight / 2)) };
    Point top_right = { (uint16_t)(start_point->x + WifiBindingWidth), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (CommonWiFiBinding::EditingPropertyId)editing_property_id
                                     == CommonWiFiBinding::EditingPropertyId::cwbepi_None
                              && Blinking_50(fb);
    if (!blink_body_on_editing) {
        ASSERT(draw_rectangle(fb,
                              top_left.x,
                              top_left.y,
                              WifiBindingWidth + 1,
                              WifiBindingHeight + 1,
                              state != LogicItemState::lisActive));
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonWiFiBinding::EditingPropertyId)editing_property_id
                                      == CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr
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
        draw_bitmap(fb,
                    start_point->x,
                    start_point->y - (GetCurrentBitmap()->size.height / 2),
                    GetCurrentBitmap());
    }
    start_point->x += GetCurrentBitmap()->size.width;

    top_right.y += (WifiBindingHeight / 2);
    EditableElement::Render(fb, &top_right);
}

size_t CommonWiFiBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t CommonWiFiBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

void CommonWiFiBinding::SelectPrior() {
    log_i(TAG_CommonWiFiBinding, "SelectPrior");

    switch (editing_property_id) {
        case CommonWiFiBinding::EditingPropertyId::cwbepi_None:
            break;
        case CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr: {
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

void CommonWiFiBinding::SelectNext() {
    log_i(TAG_CommonWiFiBinding, "SelectNext");

    switch (editing_property_id) {
        case CommonWiFiBinding::EditingPropertyId::cwbepi_None:
            break;
        case CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr: {
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

void CommonWiFiBinding::PageUp() {
}

void CommonWiFiBinding::PageDown() {
}

void CommonWiFiBinding::Change() {
    switch (editing_property_id) {
        case CommonWiFiBinding::EditingPropertyId::cwbepi_None:
            editing_property_id = CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr;
            break;

        default:
            break;
    }
}
void CommonWiFiBinding::Option() {
}

const Bitmap *CommonWiFiBinding::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}