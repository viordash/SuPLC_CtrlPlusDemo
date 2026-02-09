#pragma once

#include "Display/InteractiveElement.h"
#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class TrackBarControl : public LogicElement,
                        public InteractiveElement,
                        public InputOutputElement,
                        public LabeledElement {
  public:
    typedef enum { //
        tbepi_None = EditableElement::EditingPropertyId::cepi_None,
        tbepi_ConfigureIOAdr
    } EditingPropertyId;

  protected:
    static const int update_period_ms = 1000;
    uint8_t pointer_val;
    bool touch_screen_pressed;
    const AllowedIO GetAllowedInputs();
    void SetPointerPosition(uint8_t eng_value);

  public:
    explicit TrackBarControl();
    explicit TrackBarControl(const MapIO io_adr);
    ~TrackBarControl();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static TrackBarControl *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
