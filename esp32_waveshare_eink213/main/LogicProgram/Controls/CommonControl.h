#pragma once

#include "Display/InteractiveElement.h"
#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonControl : public LogicElement, public InteractiveElement {
  public:
    typedef enum { //
        ciepi_None = EditableElement::EditingPropertyId::cepi_None
    } EditingPropertyId;

  protected:
    LogicItemState prev_elem_state;
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    explicit CommonControl();
    virtual ~CommonControl();

    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    static CommonControl *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
