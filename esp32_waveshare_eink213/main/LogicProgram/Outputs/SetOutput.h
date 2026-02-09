#pragma once

#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class SetOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedOutputs() override final;

  protected:
  public:
    explicit SetOutput();
    explicit SetOutput(const MapIO io_adr);
    ~SetOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    TvElementType GetElementType() override final;

    static SetOutput *TryToCast(CommonOutput *common_output);
};
