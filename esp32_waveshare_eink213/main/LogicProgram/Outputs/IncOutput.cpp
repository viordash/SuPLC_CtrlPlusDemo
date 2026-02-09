#include "LogicProgram/Outputs/IncOutput.h"
#include "icons/inc_output.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_IncOutput = "IncOutput";

IncOutput::IncOutput() : CommonOutput() {
}

IncOutput::IncOutput(const MapIO io_adr) : IncOutput() {
    SetIoAdr(io_adr);
}

IncOutput::~IncOutput() {
}

bool IncOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
            uint8_t prev_val = Input->PeekValue();
            if (prev_val < LogicElement::MaxValue) {
                prev_val++;
            }
            Output->WriteValue(prev_val);
        }
        any_changes = true;
        log_d(TAG_IncOutput, ". %u", Input->PeekValue());
    }

    return any_changes;
}

const Bitmap *IncOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType IncOutput::GetElementType() {
    return TvElementType::et_IncOutput;
}

IncOutput *IncOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_IncOutput:
            return static_cast<IncOutput *>(common_output);

        default:
            return NULL;
    }
}