#pragma once

#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonWiFiBinding : public LogicElement, public InputElement, public LabeledElement {
  protected:
    virtual const AllowedIO GetAllowedInputs() = 0;
    const Bitmap *GetCurrentBitmap();

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_ConfigureIOAdr
    } EditingPropertyId;

    explicit CommonWiFiBinding();
    explicit CommonWiFiBinding(const MapIO io_adr);
    virtual ~CommonWiFiBinding();

    void SetIoAdr(const MapIO io_adr) override final;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
