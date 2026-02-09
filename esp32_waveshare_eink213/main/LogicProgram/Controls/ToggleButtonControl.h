#pragma once

#include "LogicProgram/Controls/CommonControl.h"
#include <stdint.h>
#include <unistd.h>

class ToggleButtonControl : public CommonControl {
  private:
    const Bitmap *GetCurrentBitmap() override final;
    InteractiveElement::State prev_interactive_state;

  public:
    explicit ToggleButtonControl();
    ~ToggleButtonControl();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static ToggleButtonControl *TryToCast(CommonControl *common_input);
};
