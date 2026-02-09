#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorNe : public CommonComparator {
  private:
    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    const AllowedIO GetAllowedInputs() override final;
    bool CompareFunction() override final;

  public:
    ComparatorNe();
    ComparatorNe(uint8_t ref_percent04, const MapIO io_adr);
    TvElementType GetElementType() override final;

    static ComparatorNe *TryToCast(CommonComparator *common_comparator);
};
