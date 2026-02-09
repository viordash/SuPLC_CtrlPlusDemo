#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerMSecs : public CommonTimer {
  private:
    const uint32_t step_ms = 50;
    const uint32_t faststep_ms = 250;
    const uint32_t min_delay_time_ms = 50;
    const uint32_t max_delay_time_ms = 99950;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;

  public:
    explicit TimerMSecs();
    explicit TimerMSecs(uint32_t delay_time_ms);
    ~TimerMSecs();

    void SetTime(uint32_t delay_time_ms);
    uint64_t GetTimeUs();

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static TimerMSecs *TryToCast(CommonTimer *common_timer);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
