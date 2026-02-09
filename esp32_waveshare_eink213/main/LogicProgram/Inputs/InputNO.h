#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class InputNO : public CommonInput {
  private:
    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedInputs() override final;

  public:
    explicit InputNO();
    explicit InputNO(const MapIO io_adr);
    ~InputNO();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static InputNO *TryToCast(CommonInput *common_input);
};
