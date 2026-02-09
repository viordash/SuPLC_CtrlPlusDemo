#include "LogicProgram/Outputs/SetOutput.h"
#include "icons/set_output.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SetOutput = "SetOutput";

SetOutput::SetOutput() : CommonOutput() {
}

SetOutput::SetOutput(const MapIO io_adr) : SetOutput() {
    SetIoAdr(io_adr);
}

SetOutput::~SetOutput() {
}

bool SetOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
        if (state == LogicItemState::lisActive) {
            Output->WriteValue(LogicElement::MaxValue);
        }
        any_changes = true;
        log_d(TAG_SetOutput, ".");
    }

    return any_changes;
}

const Bitmap *SetOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType SetOutput::GetElementType() {
    return TvElementType::et_SetOutput;
}

SetOutput *SetOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_SetOutput:
            return static_cast<SetOutput *>(common_output);

        default:
            return NULL;
    }
}