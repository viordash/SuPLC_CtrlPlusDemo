#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "LogicProgram/Controller.h"
#include "board.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
    view_top_index = 0;
    frame_buffer_req_render = false;
}

Ladder::~Ladder() {
    RemoveAll();
}

void Ladder::RemoveAll() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        log_d(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
    SetViewTopIndex(0);
}

bool Ladder::DoAction() {
    bool any_changes = false;
    for (auto it = begin(); it != end(); ++it) {
        any_changes |= (*it)->DoAction();
    }
    return any_changes;
}

IRAM_ATTR void Ladder::Render(FrameBuffer *fb) {
    int top_index = GetViewTopIndex();
    auto view_count = size();
    for (size_t i = top_index; i < view_count; i++) {
        uint8_t network_number = i - top_index;
        if (network_number >= Ladder::MaxViewPortCount) {
            break;
        }
        at(i)->Render(fb, i - top_index);
    }

    ScrollBar::Render(fb, view_count, Ladder::MaxViewPortCount, top_index);

    fb->has_changes |= frame_buffer_req_render || Controller::InDesign();
    fb->view_offset = top_index;
    fb->view_count = view_count;
    frame_buffer_req_render = false;
}

void Ladder::Append(Network *network) {
    log_d(TAG_Ladder, "append network: %p", network);
    push_back(network);
    frame_buffer_req_render = true;
}

void Ladder::Duplicate(int network_id) {
    log_d(TAG_Ladder, "duplicate network id: %d", network_id);

    size_t size = (*this)[network_id]->Serialize(NULL, 0);
    if (size == 0) {
        log_e(TAG_Ladder, "Duplicate error");
        return;
    }
    uint8_t *data = new uint8_t[size];

    if ((*this)[network_id]->Serialize(data, size) != size) {
        log_e(TAG_Ladder, "Duplicate serialize error");
        delete[] data;
        return;
    }

    auto new_network = new Network();
    size_t network_readed = new_network->Deserialize(data, size);
    delete[] data;
    if (network_readed == 0) {
        log_e(TAG_Ladder, "Duplicate deserialize error");
        delete new_network;
        return;
    }

    auto pos = begin();
    insert(std::next(pos, network_id), new_network);
    frame_buffer_req_render = true;
}

void Ladder::Delete(int network_id) {
    auto pos = begin();
    auto it = std::next(pos, network_id);
    auto network = *it;
    erase(it);
    delete network;
    frame_buffer_req_render = true;
}

void Ladder::SetViewTopIndex(int32_t index) {
    log_i(TAG_Ladder, "SetViewTopIndex, index:%d", (int)index);
    if (index < 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(lock_view_top_index);
    view_top_index = index;
    frame_buffer_req_render = true;
}

int32_t Ladder::GetViewTopIndex() {
    std::lock_guard<std::mutex> lock(lock_view_top_index);
    return view_top_index;
}

void Ladder::SetSelectedNetworkIndex(int32_t index) {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    int32_t top_index = GetViewTopIndex();
    log_i(TAG_Ladder,
          "SetSelectedNetworkIndex, %u, view_top_index:%u, selected_network:%d, index:%d",
          (unsigned)design_state,
          (unsigned)top_index,
          selected_network,
          (int)index);
    if (index < 0 || index >= (int)size()) {
        return;
    }

    index = std::clamp(index, top_index, (top_index + (int32_t)Ladder::MaxViewPortCount) - 1);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            (*this)[index]->Select();
            Controller::DesignStart();
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            (*this)[index]->Select();
            break;

        case EditableElement::ElementState::des_Editing:
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            break;

        case EditableElement::ElementState::des_Moving:
            break;

        case EditableElement::ElementState::des_Copying:
            break;

        case EditableElement::ElementState::des_Deleting:
            break;

        case EditableElement::ElementState::des_Disabling:
            break;
    }
}

void Ladder::AtLeastOneNetwork() {
    if (!empty()) {
        return;
    }
    log_i(TAG_Ladder, "requires at least one network");
    HandleButtonSelect();
}
