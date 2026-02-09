#pragma once

#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class ResetOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedOutputs() override final;

  protected:
  public:
    explicit ResetOutput();
    explicit ResetOutput(const MapIO io_adr);
    ~ResetOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    TvElementType GetElementType() override final;

    static ResetOutput *TryToCast(CommonOutput *common_output);
};
