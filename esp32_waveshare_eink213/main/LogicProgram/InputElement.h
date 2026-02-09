#pragma once

#include "Display/Common.h"
#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogicElement;
class InputElement {
  protected:
    template <class T> bool FindIoAdr(T container, MapIO *io_adr);

  public:
    InputElement();
    virtual ~InputElement();
    virtual void SetIoAdr(const MapIO io_adr);
    MapIO GetIoAdr();

    ControllerBaseInput *Input;

    static InputElement *TryToCast(LogicElement *logic_element);
};
