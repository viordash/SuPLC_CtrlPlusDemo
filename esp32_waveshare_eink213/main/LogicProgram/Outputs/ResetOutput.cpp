#include "LogicProgram/Outputs/ResetOutput.h"
#include "icons/reset_output.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ResetOutput = "ResetOutput";

ResetOutput::ResetOutput() : CommonOutput() {
}

ResetOutput::ResetOutput(const MapIO io_adr) : ResetOutput() {
    SetIoAdr(io_adr);
}

ResetOutput::~ResetOutput() {
}

bool ResetOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
            Output->WriteValue(LogicElement::MinValue);
        }
        any_changes = true;
        log_d(TAG_ResetOutput, ".");
    }

    return any_changes;
}

const Bitmap *ResetOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType ResetOutput::GetElementType() {
    return TvElementType::et_ResetOutput;
}

ResetOutput *ResetOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_ResetOutput:
            return static_cast<ResetOutput *>(common_output);

        default:
            return NULL;
    }
}