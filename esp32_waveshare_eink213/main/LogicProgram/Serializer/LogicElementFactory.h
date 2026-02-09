#pragma once

#include "LogicProgram/LogicElement.h"
#include "LogicProgram/Serializer/TypeValueElement.h"
#include <stdint.h>
#include <unistd.h>

class LogicElementFactory {
  protected:
  public:
    static LogicElement *Create(TvElementType element_type);
};
