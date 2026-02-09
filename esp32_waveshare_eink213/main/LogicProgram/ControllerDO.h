#pragma once

#include "LogicProgram/ControllerBaseInputOutput.h"
#include "board.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDO : public ControllerBaseInputOutput {
  protected:
    gpio_output gpio;

  public:
    explicit ControllerDO(gpio_output gpio, const char *name);

    void FetchValue() override;
    void CommitChanges() override;
};
