#include "Display/EditableElement.h"
#include "Display/display.h"
#include "EditableElement.h"
#include "LogicProgram/Controller.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const Bitmap *ElementCursor_0;
extern const Bitmap *ElementCursor_1;
extern const Bitmap *ElementCursor_2;
extern const Bitmap *CursorMovingUpDown_0;
extern const Bitmap *CursorMovingUpDown_1;
extern const Bitmap *CursorCopying_0;
extern const Bitmap *CursorCopying_1;
extern const Bitmap *CursorDeleting_0;
extern const Bitmap *CursorDeleting_1;
extern const Bitmap *CursorDisabling_0;
extern const Bitmap *CursorDisabling_1;

EditableElement::EditableElement() {
    editable_state = EditableElement::ElementState::des_Regular;
    editing_property_id = EditingPropertyId::cepi_None;
    blink = false;
}

EditableElement::~EditableElement() {
}

void EditableElement::Select() {
    editable_state = EditableElement::ElementState::des_Selected;
}
void EditableElement::CancelSelection() {
    editable_state = EditableElement::ElementState::des_Regular;
}

void EditableElement::BeginEditing() {
    editable_state = EditableElement::ElementState::des_Editing;
    editing_property_id = EditingPropertyId::cepi_None;
}
void EditableElement::EndEditing() {
    editable_state = EditableElement::ElementState::des_Regular;
}

bool EditableElement::Selected() {
    return editable_state == EditableElement::ElementState::des_Selected;
}

void EditableElement::Render(FrameBuffer *fb, Point *start_point) {
    const Bitmap *bitmap = GetCursorBitmap(fb);
    if (bitmap != NULL) {
        draw_bitmap(fb,
                    start_point->x + CursorLeftPadding,
                    start_point->y + CursorTopPadding,
                    bitmap);
    }
}

const Bitmap *EditableElement::GetCursorBitmap(FrameBuffer *fb) {
    switch (editable_state) {
        case EditableElement::ElementState::des_Selected:
            return Blinking_50(fb) ? ElementCursor_0 : ElementCursor_1;

        case EditableElement::ElementState::des_Editing:
            return ElementCursor_2;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            return Blinking_50(fb) ? CursorMovingUpDown_0 : CursorMovingUpDown_1;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            return Blinking_50(fb) ? CursorCopying_0 : CursorCopying_1;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            return Blinking_50(fb) ? CursorDeleting_0 : CursorDeleting_1;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            return Blinking_50(fb) ? CursorDisabling_0 : CursorDisabling_1;

        case EditableElement::ElementState::des_Moving:
            return CursorMovingUpDown_0;

        case EditableElement::ElementState::des_Copying:
            return CursorCopying_0;

        case EditableElement::ElementState::des_Deleting:
            return CursorDeleting_0;

        case EditableElement::ElementState::des_Disabling:
            return CursorDisabling_0;

        default:
            return NULL;
    }
}

uint16_t EditableElement::GetCursorWidth(FrameBuffer *fb) {
    const Bitmap *bitmap = GetCursorBitmap(fb);
    if (bitmap != NULL) {
        return bitmap->size.width;
    }
    return 0;
}

EditableElement::ElementState EditableElement::GetEditable_state() {
    return editable_state;
}

bool EditableElement::Editing() {
    return editable_state == EditableElement::ElementState::des_Editing;
}

bool EditableElement::InEditingProperty() {
    return editing_property_id != EditableElement::EditingPropertyId::cepi_None;
}

bool EditableElement::Blinking_50(FrameBuffer *fb) {
    const int blink_timer_ms = 400;

    const void *blinking_id = &blink;
    auto blinking = Controller::RequestWakeupMs(blinking_id,
                                                blink_timer_ms,
                                                ProcessWakeupRequestPriority::pwrp_Idle);
    if (blinking) {
        Controller::WakeupProcessTask();
        blink = !blink;
        if (fb != FRAMEBUFFER_STUB) {
            fb->has_changes = true;
        }
    }
    return blink;
}
