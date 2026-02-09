#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerSecs : public CommonTimer {
  private:
    const static uint64_t force_render_period_us = 1000000;

    const uint32_t step_s = 1;
    const uint32_t faststep_s = 10;
    const uint32_t min_delay_time_s = 1;
    const uint32_t max_delay_time_s = 99999;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;

  protected:
  public:
    explicit TimerSecs();
    explicit TimerSecs(uint32_t delay_time_s);
    ~TimerSecs();

    void SetTime(uint32_t delay_time_s);
    uint64_t GetTimeUs();

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static TimerSecs *TryToCast(CommonTimer *common_timer);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
