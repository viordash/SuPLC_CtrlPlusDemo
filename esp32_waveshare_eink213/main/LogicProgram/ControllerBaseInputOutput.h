#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInputOutput : public ControllerBaseInput {
  protected:
    uint8_t out_value;
    bool required_writing;

  public:
    ControllerBaseInputOutput(const char *name);
    virtual ~ControllerBaseInputOutput();

    virtual void Init() override;
    virtual void CommitChanges() = 0;
    void WriteValue(uint8_t new_value);
};
