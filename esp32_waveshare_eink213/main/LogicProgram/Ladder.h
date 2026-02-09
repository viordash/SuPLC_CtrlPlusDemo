#pragma once
#include "Display/EditableElement.h"
#include "Display/display.h"
#include "LogicProgram/Network.h"
#include "partitions.h"
#include <mutex>
#include <stdint.h>
#include <unistd.h>
#include <vector>

#define TAG_Ladder "Ladder"

#define LADDER_VERSION ((uint32_t)0x20240905)

class Ladder : public std::vector<Network *> {
  protected:
    std::mutex lock_view_top_index;
    int32_t view_top_index;
    bool frame_buffer_req_render;

    size_t Deserialize(uint8_t *buffer, size_t buffer_size);
    size_t Serialize(uint8_t *buffer, size_t buffer_size);

    void RemoveAll();

    EditableElement::ElementState GetDesignState(int selected_network);
    int GetSelectedNetwork();
    bool RemoveNetworkIfEmpty(int network_id);
    void InitialLoad();

  public:
    const size_t MinNetworksCount = Ladder_MinNetworksCount;
    const size_t MaxNetworksCount = Ladder_MaxNetworksCount;
    const size_t MaxViewPortCount = MAX_VIEW_PORT_COUNT;

    explicit Ladder();
    ~Ladder();

    bool DoAction();
    void Render(FrameBuffer *fb);
    void AtLeastOneNetwork();
    void Append(Network *network);
    void Duplicate(int network_id);
    void Delete(int network_id);
    bool ScrollUp(int *selected_network);
    bool ScrollDown(int *selected_network);
    void HandleButtonUp();
    void HandleButtonPageUp();
    void HandleButtonDown();
    void HandleButtonPageDown();
    void HandleButtonSelect();
    void HandleButtonOption();

    void Load();
    void Store();
    static void DeleteStorage();

    void SetViewTopIndex(int32_t index);
    int32_t GetViewTopIndex();
    void SetSelectedNetworkIndex(int32_t index);
};
