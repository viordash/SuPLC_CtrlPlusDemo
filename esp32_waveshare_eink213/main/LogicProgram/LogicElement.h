#pragma once

#include "Display/Common.h"
#include "Display/EditableElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include "LogicProgram/Serializer/TypeValueElement.h"
#include <mutex>
#include <stdint.h>
#include <unistd.h>

class Network;
class ElementsBox;

class LogicElement : public EditableElement {
  protected:
    std::mutex lock_mutex;
    LogicItemState state;

    friend Network;
    friend ElementsBox;

  public:
    static const uint8_t MinValue = 0;
    static const uint8_t MaxValue = 255;

    LogicElement();
    virtual ~LogicElement();

    LogicItemState GetState();

    virtual bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) = 0;
    virtual void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) = 0;

    virtual size_t Serialize(uint8_t *buffer, size_t buffer_size) = 0;
    virtual size_t Deserialize(uint8_t *buffer, size_t buffer_size) = 0;

    virtual TvElementType GetElementType() = 0;
};
