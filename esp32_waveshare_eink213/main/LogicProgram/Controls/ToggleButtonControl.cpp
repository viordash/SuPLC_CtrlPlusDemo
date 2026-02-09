#include "LogicProgram/Controls/ToggleButtonControl.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/toggle_button.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUPPORT_INTERACTIVE

static const char *TAG_ToggleButtonControl = "ToggleButtonControl";

ToggleButtonControl::ToggleButtonControl() : CommonControl() {
    prev_interactive_state = InteractiveElement::State::csReleased;
}

ToggleButtonControl::~ToggleButtonControl() {
}

bool ToggleButtonControl::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    this->prev_elem_state = prev_elem_state;
    LogicItemState prev_state = state;
    if (prev_elem_state == LogicItemState::lisActive) {
        DetermineInteractive();
        bool interactive_changed = prev_interactive_state != interactive_state;
        prev_interactive_state = interactive_state;
        if (interactive_changed && interactive_state == InteractiveElement::State::csPressed) {
            switch (state) {
                case LogicItemState::lisActive:
                    state = LogicItemState::lisPassive;
                    break;

                case LogicItemState::lisPassive:
                    state = LogicItemState::lisActive;
                    break;
            }
        }
    } else {
        state = LogicItemState::lisPassive;
        prev_interactive_state = InteractiveElement::State::csReleased;
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_ToggleButtonControl, ".");
    }
    return any_changes;
}

const Bitmap *ToggleButtonControl::GetCurrentBitmap() {
    switch (prev_elem_state) {
        case LogicItemState::lisActive:
            switch (state) {
                case LogicItemState::lisActive:
                    return &icon_pressed;
                default:
                    return &icon_active;
            }

        default:
            return &icon_passive;
    }
}

size_t ToggleButtonControl::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    uint8_t dummy = 0;
    if (!Record::Write(&dummy, sizeof(dummy), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t ToggleButtonControl::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t dummy;
    if (!Record::Read(&dummy, sizeof(dummy), buffer, buffer_size, &readed)) {
        return 0;
    }
    return readed;
}

TvElementType ToggleButtonControl::GetElementType() {
    return TvElementType::et_ToggleButtonControl;
}

ToggleButtonControl *ToggleButtonControl::TryToCast(CommonControl *common_input) {
    switch (common_input->GetElementType()) {
        case TvElementType::et_ToggleButtonControl:
            return static_cast<ToggleButtonControl *>(common_input);

        default:
            return NULL;
    }
}

#endif //SUPPORT_INTERACTIVE