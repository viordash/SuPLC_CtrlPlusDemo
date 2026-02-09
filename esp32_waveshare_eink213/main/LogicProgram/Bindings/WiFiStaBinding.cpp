#include "LogicProgram/Bindings/WiFiStaBinding.h"
#include "DeviceIO.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiStaBinding = "WiFiStaBinding";

WiFiStaBinding::WiFiStaBinding() : CommonWiFiBinding() {
}

WiFiStaBinding::WiFiStaBinding(const MapIO io_adr) : CommonWiFiBinding(io_adr) {
}

WiFiStaBinding::~WiFiStaBinding() {
}

bool WiFiStaBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        DeviceIO::BindVariableToStaWiFi(GetIoAdr());
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        DeviceIO::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_WiFiStaBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR void
WiFiStaBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    Point draw_point = *start_point;
    CommonWiFiBinding::Render(fb, prev_elem_state, &draw_point);

    draw_point.x += body_font.GetRightLeftMargin();
    ASSERT(
        body_font.DrawText(fb, draw_point.x, draw_point.y - body_font.GetHeight() / 2, "STA CLNT")
        > 0);

    start_point->x += WifiBindingLeftPadding + WifiBindingWidth;
}

TvElementType WiFiStaBinding::GetElementType() {
    return TvElementType::et_WiFiStaBinding;
}

WiFiStaBinding *WiFiStaBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiStaBinding:
            return static_cast<WiFiStaBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiStaBinding::Change() {
    log_i(TAG_WiFiStaBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiStaBinding::EditingPropertyId::wsbepi_None:
            CommonWiFiBinding::Change();
            break;

        default:
            EndEditing();
            break;
    }
}
