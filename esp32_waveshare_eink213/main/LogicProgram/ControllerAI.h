#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

class ControllerAI : public ControllerBaseInput {
  protected:
    adc_input adc;

  public:
    explicit ControllerAI(adc_input adc, const char *name);
    void FetchValue() override;
};
