#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class DecOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedOutputs() override final;

  public:
    explicit DecOutput();
    explicit DecOutput(const MapIO io_adr);
    ~DecOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    TvElementType GetElementType() override final;

    static DecOutput *TryToCast(CommonOutput *common_output);
};
