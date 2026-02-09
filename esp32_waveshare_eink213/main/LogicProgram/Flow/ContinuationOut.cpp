#include "LogicProgram/Flow/ContinuationOut.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "icons/continuation_out.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ContinuationOut = "ContinuationOut";

ContinuationOut::ContinuationOut() : CommonContinuation() {
}

ContinuationOut::~ContinuationOut() {
}

bool ContinuationOut::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);

    auto network_continuation = Controller::GetNetworkContinuation();

    log_d(TAG_ContinuationOut, "DoAction: continuation out, state:%u", network_continuation);

    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = network_continuation;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_ContinuationOut, ".");
    }

    return any_changes;
}

IRAM_ATTR void
ContinuationOut::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    std::lock_guard<std::mutex> lock(lock_mutex);

    start_point->x += ContinuationOutLeftPadding;

    auto bitmap = GetCurrentBitmap();
    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (EditableElement::EditingPropertyId)editing_property_id
                                       == EditableElement::EditingPropertyId::cepi_None
                                && Blinking_50(fb);
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, bitmap_top, bitmap);
    }

    start_point->x += bitmap->size.width;

    EditableElement::Render(fb, start_point);
}

const Bitmap *ContinuationOut::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

TvElementType ContinuationOut::GetElementType() {
    return TvElementType::et_ContinuationOut;
}

ContinuationOut *ContinuationOut::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ContinuationOut:
            return static_cast<ContinuationOut *>(logic_element);

        default:
            return NULL;
    }
}