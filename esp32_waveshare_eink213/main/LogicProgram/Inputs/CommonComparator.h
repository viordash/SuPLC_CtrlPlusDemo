#pragma once

#include "Display/Font.h"
#include "Display/display.h"
#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class CommonComparator : public CommonInput {
  public:
    typedef enum { //
        ccepi_None = CommonInput::EditingPropertyId::ciepi_None,
        ccepi_ConfigureIoAdr = CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr,
        ccepi_ConfigureReference
    } EditingPropertyId;

  protected:
    int str_size;
    char str_reference[5];
    uint8_t ref_percent04;

    static const uint8_t step_ref = 1;
    static const uint8_t faststep_ref = 10;

    FontMedium reference_font_1;
    FontMedium reference_font_2;
    FontSmall reference_font_3;

    virtual bool CompareFunction() = 0;

  public:
    CommonComparator();
    CommonComparator(uint8_t ref_percent04, const MapIO io_adr);
    virtual ~CommonComparator();

    void SetReference(uint8_t ref_percent04);
    uint8_t GetReference();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;

    static CommonComparator *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
