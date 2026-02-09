#include "LogicProgram/Outputs/CommonOutput.h"
#include "Display/display.h"
#include "LogicProgram/Outputs/DecOutput.h"
#include "LogicProgram/Outputs/DirectOutput.h"
#include "LogicProgram/Outputs/IncOutput.h"
#include "LogicProgram/Outputs/ResetOutput.h"
#include "LogicProgram/Outputs/SetOutput.h"
#include "LogicProgram/Serializer/Record.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonOutput = "CommonOutput";

CommonOutput::CommonOutput() : LogicElement(), InputOutputElement() {
}

CommonOutput::~CommonOutput() {
}

void CommonOutput::SetIoAdr(const MapIO io_adr) {
    InputOutputElement::SetIoAdr(io_adr);
    SetLabel(DeviceIO::GetIOName(io_adr));
}

IRAM_ATTR void
CommonOutput::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    std::lock_guard<std::mutex> lock(lock_mutex);

    uint16_t line_w = label_width + OutputRightPadding + label_font.GetRightLeftMargin();
    start_point->x -= line_w;
    if (state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, line_w));
    } else {
        ASSERT(draw_passive_network(fb, start_point->x, start_point->y, line_w, true));
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonOutput::EditingPropertyId)editing_property_id
                                      == CommonOutput::EditingPropertyId::coepi_ConfigureOutputAdr
                               && Blinking_50(fb);
    if (!blink_label_on_editing) {
        ASSERT(
            label_font.DrawText(fb, start_point->x, start_point->y - label_font.GetHeight(), label)
            > 0);
    }

    EditableElement::Render(fb, start_point);

    auto bitmap = GetCurrentBitmap(state);
    start_point->x -= bitmap->size.width;
    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonOutput::EditingPropertyId)editing_property_id
                                       == CommonOutput::EditingPropertyId::coepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }
}

size_t CommonOutput::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t CommonOutput::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

CommonOutput *CommonOutput::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_DirectOutput:
        case TvElementType::et_SetOutput:
        case TvElementType::et_ResetOutput:
        case TvElementType::et_IncOutput:
        case TvElementType::et_DecOutput:
            return static_cast<CommonOutput *>(logic_element);

        default:
            return NULL;
    }
}

void CommonOutput::SelectPrior() {
    log_i(TAG_CommonOutput, "SelectPrior");

    auto allowed_outputs = GetAllowedOutputs();
    auto io_adr = FindAllowedIO(&allowed_outputs, GetIoAdr());
    io_adr--;
    if (io_adr < 0) {
        io_adr = allowed_outputs.count - 1;
    }
    SetIoAdr(allowed_outputs.inputs_outputs[io_adr]);
}

void CommonOutput::SelectNext() {
    log_i(TAG_CommonOutput, "SelectNext");

    auto allowed_outputs = GetAllowedOutputs();
    auto io_adr = FindAllowedIO(&allowed_outputs, GetIoAdr());
    io_adr++;
    if (io_adr >= (int)allowed_outputs.count) {
        io_adr = 0;
    }
    SetIoAdr(allowed_outputs.inputs_outputs[io_adr]);
}

void CommonOutput::PageUp() {
}
void CommonOutput::PageDown() {
}

void CommonOutput::Change() {
    log_i(TAG_CommonOutput, "Change");
    switch (editing_property_id) {
        case CommonOutput::EditingPropertyId::coepi_None:
            editing_property_id = CommonOutput::EditingPropertyId::coepi_ConfigureOutputAdr;
            break;

        default:
            EndEditing();
            break;
    }
}

void CommonOutput::Option() {
}