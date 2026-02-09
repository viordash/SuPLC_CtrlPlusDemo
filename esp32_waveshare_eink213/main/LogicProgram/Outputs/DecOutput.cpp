#include "LogicProgram/Outputs/DecOutput.h"
#include "icons/dec_output.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DecOutput = "DecOutput";

DecOutput::DecOutput() : CommonOutput() {
}

DecOutput::DecOutput(const MapIO io_adr) : DecOutput() {
    SetIoAdr(io_adr);
}

DecOutput::~DecOutput() {
}

bool DecOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
            if (prev_val > LogicElement::MinValue) {
                prev_val--;
            }
            Output->WriteValue(prev_val);
        }
        any_changes = true;
        log_d(TAG_DecOutput, ". %u", Input->PeekValue());
    }

    return any_changes;
}

const Bitmap *DecOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType DecOutput::GetElementType() {
    return TvElementType::et_DecOutput;
}

DecOutput *DecOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_DecOutput:
            return static_cast<DecOutput *>(common_output);

        default:
            return NULL;
    }
}