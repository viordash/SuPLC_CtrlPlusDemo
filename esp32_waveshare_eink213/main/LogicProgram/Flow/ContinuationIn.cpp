#include "LogicProgram/Flow/ContinuationIn.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/continuation_in.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ContinuationIn = "ContinuationIn";

ContinuationIn::ContinuationIn() : CommonContinuation() {
}

ContinuationIn::~ContinuationIn() {
}

bool ContinuationIn::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    Controller::SetNetworkContinuation(prev_elem_state);

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
        any_changes = true;
        log_d(TAG_ContinuationIn, ".");
    }

    return any_changes;
}

IRAM_ATTR void
ContinuationIn::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    std::lock_guard<std::mutex> lock(lock_mutex);

    start_point->x -= ContinuationInInRightPadding;

    auto bitmap = GetCurrentBitmap();
    start_point->x -= bitmap->size.width;

    auto cursor_width = GetCursorWidth(fb);
    start_point->x -= cursor_width;

    EditableElement::Render(fb, start_point);
    start_point->x += cursor_width;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (EditableElement::EditingPropertyId)editing_property_id
                                       == EditableElement::EditingPropertyId::cepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }
}

const Bitmap *ContinuationIn::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType ContinuationIn::GetElementType() {
    return TvElementType::et_ContinuationIn;
}

ContinuationIn *ContinuationIn::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ContinuationIn:
            return static_cast<ContinuationIn *>(logic_element);

        default:
            return NULL;
    }
}