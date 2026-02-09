#pragma once

#include "Display/Common.h"
#include "board.h"
#include "config.h"
#include <stdint.h>
#include <unistd.h>

class EditableElement {

  public:
    typedef enum { //
        des_Regular,
        des_Selected,
        des_Editing,
        des_AdvancedSelectMove,
        des_AdvancedSelectCopy,
        des_AdvancedSelectDelete,
        des_AdvancedSelectDisable,
        des_Moving,
        des_Copying,
        des_Deleting,
        des_Disabling
    } ElementState;

    typedef enum { //
        cepi_None = 0
    } EditingPropertyId;

  protected:
    EditableElement::ElementState editable_state;
    int editing_property_id;
    bool blink;

    void Render(FrameBuffer *fb, Point *start_point);

    bool Blinking_50(FrameBuffer *fb);
    const Bitmap *GetCursorBitmap(FrameBuffer *fb);
    uint16_t GetCursorWidth(FrameBuffer *fb);

  public:
    explicit EditableElement();
    virtual ~EditableElement();

    void Select();
    void CancelSelection();

    virtual void BeginEditing();
    virtual void EndEditing();

    EditableElement::ElementState GetEditable_state();
    bool Selected();
    bool Editing();
    bool InEditingProperty();

    virtual void SelectPrior() = 0;
    virtual void SelectNext() = 0;
    virtual void PageUp() = 0;
    virtual void PageDown() = 0;
    virtual void Change() = 0;
    virtual void Option() = 0;
};
