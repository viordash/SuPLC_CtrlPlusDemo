#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDI : public ControllerBaseInput {
  protected:
  protected:
    gpio_input gpio;

  public:
    explicit ControllerDI(gpio_input gpio, const char *name);
    void FetchValue() override;
};
