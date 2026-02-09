#include "LogicProgram/Controls/CommonControl.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUPPORT_INTERACTIVE

static const char *TAG_CommonInput = "CommonControl";

CommonControl::CommonControl() : LogicElement(), InteractiveElement() {
    prev_elem_state = LogicItemState::lisPassive;
}

CommonControl::~CommonControl() {
}

IRAM_ATTR void
CommonControl::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    uint16_t line_w = ButtonControlLeftPadding;
    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, line_w));
    } else {
        ASSERT(draw_passive_network(fb, start_point->x, start_point->y, line_w, false));
    }

    start_point->x += ButtonControlLeftPadding;

    auto bitmap = GetCurrentBitmap();

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (bitmap->size.height / 2)) };
    Point bottom_right = { (uint16_t)(top_left.x + bitmap->size.width),
                           (uint16_t)(top_left.y + bitmap->size.height) };
    SetInteractiveArea(&top_left, &bottom_right);

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonControl::EditingPropertyId)editing_property_id
                                       == CommonControl::EditingPropertyId::ciepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }

    start_point->x += bitmap->size.width;

    EditableElement::Render(fb, start_point);
}

CommonControl *CommonControl::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ButtonControl:
        case TvElementType::et_ToggleButtonControl:
            return static_cast<CommonControl *>(logic_element);

        default:
            return NULL;
    }
}

void CommonControl::SelectPrior() {
    log_i(TAG_CommonInput, "SelectPrior");
}

void CommonControl::SelectNext() {
    log_i(TAG_CommonInput, "SelectNext");
}

void CommonControl::PageUp() {
}

void CommonControl::PageDown() {
}

void CommonControl::Change() {
    log_i(TAG_CommonInput, "Change");
    switch (editing_property_id) {
        default:
            EndEditing();
            break;
    }
}

void CommonControl::Option() {
}

#endif //SUPPORT_INTERACTIVE