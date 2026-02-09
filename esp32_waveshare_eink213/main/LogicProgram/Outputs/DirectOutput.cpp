#include "LogicProgram/Outputs/DirectOutput.h"
#include "icons/direct_output.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DirectOutput = "DirectOutput";

DirectOutput::DirectOutput() : CommonOutput() {
}

DirectOutput::DirectOutput(const MapIO io_adr) : DirectOutput() {
    SetIoAdr(io_adr);
}

DirectOutput::~DirectOutput() {
}

bool DirectOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        Output->WriteValue(state == LogicItemState::lisActive ? LogicElement::MaxValue
                                                              : LogicElement::MinValue);
        any_changes = true;
        log_d(TAG_DirectOutput, ".");
    }

    return any_changes;
}

const Bitmap *DirectOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType DirectOutput::GetElementType() {
    return TvElementType::et_DirectOutput;
}

DirectOutput *DirectOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_DirectOutput:
            return static_cast<DirectOutput *>(common_output);

        default:
            return NULL;
    }
}