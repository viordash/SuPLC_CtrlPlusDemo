#pragma once

#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonInput : public LogicElement, public InputElement, public LabeledElement {
  public:
    typedef enum { //
        ciepi_None = EditableElement::EditingPropertyId::cepi_None,
        ciepi_ConfigureInputAdr
    } EditingPropertyId;

  protected:
    virtual const Bitmap *GetCurrentBitmap(LogicItemState state) = 0;
    virtual const AllowedIO GetAllowedInputs() = 0;

  public:
    explicit CommonInput();
    virtual ~CommonInput();

    void SetIoAdr(const MapIO io_adr) override final;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    static CommonInput *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
