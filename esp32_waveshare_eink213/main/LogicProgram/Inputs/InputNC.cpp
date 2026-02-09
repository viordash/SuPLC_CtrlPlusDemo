#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/input_close.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNC = "InputNC";

InputNC::InputNC() : CommonInput() {
}

InputNC::InputNC(const MapIO io_adr) : InputNC() {
    SetIoAdr(io_adr);
}

InputNC::~InputNC() {
}

bool InputNC::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    state = LogicItemState::lisPassive;
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisPassive) {
        Input->CancelReadingProcess();
    } else if (prev_elem_state == LogicItemState::lisActive //
               && Input->ReadValue() == LogicElement::MinValue) {
        state = LogicItemState::lisActive;
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_InputNC, ".");
    }
    return any_changes;
}

const Bitmap *InputNC::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

size_t InputNC::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t InputNC::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

TvElementType InputNC::GetElementType() {
    return TvElementType::et_InputNC;
}

InputNC *InputNC::TryToCast(CommonInput *common_input) {
    switch (common_input->GetElementType()) {
        case TvElementType::et_InputNC:
            return static_cast<InputNC *>(common_input);

        default:
            return NULL;
    }
}
