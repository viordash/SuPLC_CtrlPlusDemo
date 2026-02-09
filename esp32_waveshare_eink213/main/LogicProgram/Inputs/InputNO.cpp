#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/input_open.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNO = "InputNO";

InputNO::InputNO() : CommonInput() {
}

InputNO::InputNO(const MapIO io_adr) : InputNO() {
    SetIoAdr(io_adr);
}

InputNO::~InputNO() {
}

bool InputNO::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    log_d(TAG_InputNO, "'%s' state:%u, val:%u", label, state, Input->PeekValue());

    state = LogicItemState::lisPassive;
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisPassive) {
        Input->CancelReadingProcess();
    } else if (prev_elem_state == LogicItemState::lisActive //
               && Input->ReadValue() != LogicElement::MinValue) {
        state = LogicItemState::lisActive;
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_InputNO, ".");
    }

    return any_changes;
}

const Bitmap *InputNO::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

size_t InputNO::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t InputNO::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

TvElementType InputNO::GetElementType() {
    return TvElementType::et_InputNO;
}

InputNO *InputNO::TryToCast(CommonInput *common_input) {
    switch (common_input->GetElementType()) {
        case TvElementType::et_InputNO:
            return static_cast<InputNO *>(common_input);

        default:
            return NULL;
    }
}