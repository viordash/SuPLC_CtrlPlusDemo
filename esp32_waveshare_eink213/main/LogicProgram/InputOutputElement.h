#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/ControllerBaseInputOutput.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputOutputElement : public InputElement {
  protected:
  public:
    explicit InputOutputElement();
    virtual ~InputOutputElement();

    ControllerBaseInputOutput *Output;

    virtual void SetIoAdr(const MapIO io_adr) override;
};
