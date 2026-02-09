#include "LogicProgram/Controls/ButtonControl.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/button.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUPPORT_INTERACTIVE

static const char *TAG_ButtonControl = "ButtonControl";

ButtonControl::ButtonControl() : CommonControl() {
}

ButtonControl::~ButtonControl() {
}

bool ButtonControl::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    this->prev_elem_state = prev_elem_state;
    LogicItemState prev_state = state;
    state = LogicItemState::lisPassive;
    if (prev_elem_state == LogicItemState::lisActive) {
        DetermineInteractive();
        if (interactive_state == InteractiveElement::State::csPressed) {
            state = LogicItemState::lisActive;
        }
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_ButtonControl, ".");
    }
    return any_changes;
}

const Bitmap *ButtonControl::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            switch (interactive_state) {
                case InteractiveElement::State::csPressed:
                    return &icon_pressed;
                default:
                    return &icon_active;
            }
        default:
            switch (prev_elem_state) {
                case LogicItemState::lisActive:
                    return &icon_active;
                default:
                    return &icon_passive;
            }
    }
}

size_t ButtonControl::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t ButtonControl::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t dummy;
    if (!Record::Read(&dummy, sizeof(dummy), buffer, buffer_size, &readed)) {
        return 0;
    }
    return readed;
}

TvElementType ButtonControl::GetElementType() {
    return TvElementType::et_ButtonControl;
}

ButtonControl *ButtonControl::TryToCast(CommonControl *common_input) {
    switch (common_input->GetElementType()) {
        case TvElementType::et_ButtonControl:
            return static_cast<ButtonControl *>(common_input);

        default:
            return NULL;
    }
}

#endif //SUPPORT_INTERACTIVE