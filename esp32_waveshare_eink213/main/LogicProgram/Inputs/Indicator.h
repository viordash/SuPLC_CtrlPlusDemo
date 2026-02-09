#pragma once

#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class Indicator : public LogicElement, public InputElement, public LabeledElement {
  public:
    typedef enum { //
        ciepi_None = EditableElement::EditingPropertyId::cepi_None,
        ciepi_ConfigureIOAdr,
        ciepi_ConfigureLowScale_0,
        ciepi_ConfigureLowScale_1,
        ciepi_ConfigureLowScale_2,
        ciepi_ConfigureLowScale_3,
        ciepi_ConfigureLowScale_4,
        ciepi_ConfigureLowScale_5,
        ciepi_ConfigureLowScale_6,
        ciepi_ConfigureLowScale_7,
        ciepi_ConfigureHighScale_0,
        ciepi_ConfigureHighScale_1,
        ciepi_ConfigureHighScale_2,
        ciepi_ConfigureHighScale_3,
        ciepi_ConfigureHighScale_4,
        ciepi_ConfigureHighScale_5,
        ciepi_ConfigureHighScale_6,
        ciepi_ConfigureHighScale_7
    } EditingPropertyId;

  protected:
    static const uint8_t max_symbols_count = 8;
    float low_scale;
    float high_scale;
    uint8_t decimal_point;
    char str_value[max_symbols_count + 1];
    char str_format[max_symbols_count + 1];

    FontXLarge value_font;
    FontLarge scale_font;

    const AllowedIO GetAllowedInputs();
    void UpdateScale();
    void PrintOutValue(uint8_t eng_value);
    void PrintLowScale();
    void AcceptLowScale();
    void PrintHighScale();
    void AcceptHighScale();
    uint8_t GetDecimalPointFromScale();
    void SelectPriorSymbol(char *symbol, char extra);
    void SelectNextSymbol(char *symbol, char extra);

    void RenderScales(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y);

  public:
    static const int update_period_ms = 1000;
    const float MinScale = -9999999.0f;
    const float MaxScale = 99999990.0f;

    explicit Indicator();
    explicit Indicator(const MapIO io_adr,
                       float high_scale,
                       float low_scale,
                       uint8_t decimal_point);
    virtual ~Indicator();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static Indicator *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;

    float GetLowScale();
    void SetLowScale(float scale);

    float GetHighScale();
    void SetHighScale(float scale);

    uint8_t GetDecimalPoint();
    void SetDecimalPoint(uint8_t point);
};
