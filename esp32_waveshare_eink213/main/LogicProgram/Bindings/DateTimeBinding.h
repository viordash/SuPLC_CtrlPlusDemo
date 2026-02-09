#pragma once

#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/Bindings/DatetimePart.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class DateTimeBinding : public LogicElement, public InputElement, public LabeledElement {
  protected:
    FontLarge datetime_font;

    const AllowedIO GetAllowedInputs();
    const char *GetDatetimePartName();
    const Bitmap *GetCurrentBitmap();

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_ConfigureIOAdr,
        cwbepi_SelectDatetimePart
    } EditingPropertyId;

    DatetimePart datetime_part;

    explicit DateTimeBinding();
    explicit DateTimeBinding(const MapIO io_adr);
    virtual ~DateTimeBinding();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;
    bool ValidateDatetimePart(DatetimePart datetime_part);
    TvElementType GetElementType() override;

    static DateTimeBinding *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
