#pragma once

#include "LogicProgram/Bindings/CommonWiFiBinding.h"
#include <stdint.h>
#include <unistd.h>

class WiFiStaBinding : public CommonWiFiBinding {
  protected:
    const AllowedIO GetAllowedInputs() override final;

    FontLarge body_font;

  public:
    typedef enum { //
        wsbepi_None = CommonWiFiBinding::EditingPropertyId::cwbepi_None,
        wsbepi_ConfigureIOAdr = CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr
    } EditingPropertyId;

    explicit WiFiStaBinding();
    explicit WiFiStaBinding(const MapIO io_adr);
    virtual ~WiFiStaBinding();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    TvElementType GetElementType() override final;

    static WiFiStaBinding *TryToCast(LogicElement *logic_element);

    void Change() override;
};
