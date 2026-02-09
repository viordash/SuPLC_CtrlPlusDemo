#pragma once

#include "Display/Common.h"
#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class CommonOutput : public LogicElement, public InputOutputElement, public LabeledElement {
  public:
    typedef enum { //
        coepi_None = EditableElement::EditingPropertyId::cepi_None,
        coepi_ConfigureOutputAdr
    } EditingPropertyId;

  protected:
    virtual const Bitmap *GetCurrentBitmap(LogicItemState state) = 0;
    virtual const AllowedIO GetAllowedOutputs() = 0;

  public:
    CommonOutput();
    ~CommonOutput();
    void SetIoAdr(const MapIO io_adr) override final;

    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;

    static CommonOutput *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
