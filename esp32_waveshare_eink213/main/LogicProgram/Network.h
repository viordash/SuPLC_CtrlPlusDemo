#pragma once

#include "Display/EditableElement.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/LogicItemState.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class Network : public std::vector<LogicElement *>, public EditableElement {
  protected:
    LogicItemState state;
    bool state_changed;
    bool frame_buffer_req_render;
    uint16_t fill_wire;

    bool EnoughSpaceForNewElement(LogicElement *new_element);
    void AddSpaceForNewElement();
    void RemoveSpaceForNewElement();
    bool HasOutputElement();
    bool OptionShowOutputElement(LogicElement *selected_element);
    bool OptionShowContinuationIn(LogicElement *selected_element);
    bool OptionShowContinuationOut(LogicElement *selected_element);

  public:
    explicit Network();
    explicit Network(LogicItemState state);
    virtual ~Network();

    void ChangeState(LogicItemState state);
    LogicItemState GetState();

    virtual bool DoAction();
    virtual void Render(FrameBuffer *fb, uint8_t network_number);

    void Append(LogicElement *element);

    size_t Serialize(uint8_t *buffer, size_t buffer_size);
    size_t Deserialize(uint8_t *buffer, size_t buffer_size);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void BeginEditing() override final;
    void EndEditing() override final;
    void Option() override final;
    int GetSelectedElement();

    void SwitchState();
    void SwitchToAdvancedSelectMove();
    void SwitchToAdvancedSelectCopy();
    void SwitchToAdvancedSelectDelete();
    void SwitchToAdvancedSelectDisable();
    void SwitchToMoving();
    void SwitchToCopying();
    void SwitchToDeleting();
    void SwitchToDisabling();
};
