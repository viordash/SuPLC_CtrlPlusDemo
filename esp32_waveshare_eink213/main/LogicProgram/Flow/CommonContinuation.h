#pragma once

#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonContinuation : public LogicElement {
  protected:
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    explicit CommonContinuation();
    ~CommonContinuation();

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};
