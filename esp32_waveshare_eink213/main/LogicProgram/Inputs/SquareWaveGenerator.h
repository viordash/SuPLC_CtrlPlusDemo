#pragma once

#include "Display/Font.h"
#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class SquareWaveGenerator : public LogicElement {
  public:
    typedef enum { //
        ctepi_None = EditableElement::EditingPropertyId::cepi_None,
        ctepi_ConfigurePeriod0,
        ctepi_ConfigurePeriod1
    } EditingPropertyId;

  private:
    const static uint64_t force_render_period_us = 1000000;

    const uint32_t step_ms = 10;
    const uint32_t faststep_ms = 250;
    const uint32_t min_period_ms = 10;
    const uint32_t max_period_ms = 99990;

    const Bitmap *GetCurrentBitmap(LogicItemState state);

  protected:
    uint64_t period0_us;
    uint64_t period1_us;
    char str_period0[16];
    char str_period1[16];
    size_t str_period0_size;
    size_t str_period1_size;

    FontSmall period_font;

    void
    SetPeriod(uint32_t period_ms, uint64_t *period_us, char *str_period, size_t *str_period_size);

  public:
    explicit SquareWaveGenerator();
    explicit SquareWaveGenerator(uint32_t period0_ms, uint32_t period1_ms);
    ~SquareWaveGenerator();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    void SetPeriod0(uint32_t period_ms);
    void SetPeriod1(uint32_t period_ms);
    uint64_t GetPeriod0Us();
    uint64_t GetPeriod1Us();

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static SquareWaveGenerator *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
