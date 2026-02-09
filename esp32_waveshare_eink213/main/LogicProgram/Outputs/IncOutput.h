#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class IncOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedOutputs() override final;

  public:
    explicit IncOutput();
    explicit IncOutput(const MapIO io_adr);
    ~IncOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    TvElementType GetElementType() override final;

    static IncOutput *TryToCast(CommonOutput *common_output);
};
