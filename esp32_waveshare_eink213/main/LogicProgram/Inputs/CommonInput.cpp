#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonInput = "CommonInput";

CommonInput::CommonInput() : LogicElement(), InputElement() {
}

CommonInput::~CommonInput() {
}

void CommonInput::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

IRAM_ATTR void
CommonInput::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    uint16_t line_w = label_width + InputLeftPadding + label_font.GetRightLeftMargin();
    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, line_w));
    } else {
        ASSERT(draw_passive_network(fb, start_point->x, start_point->y, line_w, false));
    }

    start_point->x += InputLeftPadding;
    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonInput::EditingPropertyId)editing_property_id
                                      == CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr
                               && Blinking_50(fb);
    if (!blink_label_on_editing) {
        ASSERT(
            label_font.DrawText(fb, start_point->x, start_point->y - label_font.GetHeight(), label)
            > 0);
    }

    start_point->x += label_font.GetRightLeftMargin();
    start_point->x += label_width;

    auto bitmap = GetCurrentBitmap(state);
    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonInput::EditingPropertyId)editing_property_id
                                       == CommonInput::EditingPropertyId::ciepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, bitmap_top, bitmap);
    }

    start_point->x += bitmap->size.width;

    EditableElement::Render(fb, start_point);
}

CommonInput *CommonInput::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_InputNC:
        case TvElementType::et_InputNO:
        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorNe:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonInput *>(logic_element);

        default:
            return NULL;
    }
}

void CommonInput::SelectPrior() {
    log_i(TAG_CommonInput, "SelectPrior");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr--;
    if (io_adr < 0) {
        io_adr = allowed_inputs.count - 1;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void CommonInput::SelectNext() {
    log_i(TAG_CommonInput, "SelectNext");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr++;
    if (io_adr >= (int)allowed_inputs.count) {
        io_adr = 0;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void CommonInput::PageUp() {
}

void CommonInput::PageDown() {
}

void CommonInput::Change() {
    log_i(TAG_CommonInput, "Change");
    switch (editing_property_id) {
        case CommonInput::EditingPropertyId::ciepi_None:
            editing_property_id = CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr;
            break;

        default:
            EndEditing();
            break;
    }
}

void CommonInput::Option() {
}