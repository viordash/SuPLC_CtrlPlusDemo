#include "LogicProgram/Bindings/WiFiBinding.h"
#include "DeviceIO.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiBinding = "WiFiBinding";

WiFiBinding::WiFiBinding() : CommonWiFiBinding() {
    ssid[0] = 0;
    ssid_size = 0;
}

WiFiBinding::WiFiBinding(const MapIO io_adr, const char *ssid) : CommonWiFiBinding(io_adr) {
    SetSsid(ssid);
}

WiFiBinding::~WiFiBinding() {
}

bool WiFiBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        DeviceIO::BindVariableToInsecureWiFi(GetIoAdr(), GetSsid());
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        DeviceIO::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_WiFiBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR void
WiFiBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    Point draw_point = *start_point;
    CommonWiFiBinding::Render(fb, prev_elem_state, &draw_point);

    draw_point.x += body_font.GetRightLeftMargin();

    bool show_edit_ssid = editable_state == EditableElement::ElementState::des_Editing
                       && (WiFiBinding::EditingPropertyId)editing_property_id
                              != WiFiBinding::EditingPropertyId::wbepi_None
                       && (WiFiBinding::EditingPropertyId)editing_property_id
                              != WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_edit_ssid) {
        RenderEditedSsid(fb,
                         draw_point.x,
                         (start_point->x + WifiBindingLeftPadding + WifiBindingWidth)
                             - value_font.GetRightLeftMargin(),
                         draw_point.y);
    } else {
        uint16_t text_y = draw_point.y - body_font.GetHeight() / 2;
        if (ssid[0] != 0) {
            ASSERT(body_font.DrawText(fb,
                                      draw_point.x,
                                      (start_point->x + WifiBindingLeftPadding + WifiBindingWidth)
                                          - body_font.GetRightLeftMargin(),
                                      text_y,
                                      ssid,
                                      FontBase::Align::Left)
                   > 0);
        }
    }

    start_point->x += WifiBindingLeftPadding + WifiBindingWidth;
}

void WiFiBinding::RenderEditedSsid(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    char editing_value[max_ssid_size + 1];
    int char_pos = editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;

    strncpy(editing_value, ssid, sizeof(editing_value) - 1);
    editing_value[sizeof(editing_value) - 1] = 0;

    if (Blinking_50(fb)) {
        editing_value[char_pos] = ' ';
    }

    uint16_t top = y - (WifiBindingHeight / 2);
    uint16_t bottom = top + WifiBindingHeight - 1;
    ASSERT(caption_font.DrawText(fb,
                                 left + caption_font.GetRightLeftMargin(),
                                 top + caption_font.GetTopMargin(),
                                 "SSID:")
           > 0);
    ASSERT(value_font.DrawLimitText(fb,
                                    left + value_font.GetRightLeftMargin(),
                                    right,
                                    bottom - value_font.GetHeight(),
                                    editing_value,
                                    &editing_value[char_pos])
           > 0);
}

size_t WiFiBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = CommonWiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    if (!Record::Write(&ssid, sizeof(ssid), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = CommonWiFiBinding::Deserialize(buffer, buffer_size);
    char _ssid[sizeof(ssid)];

    if (readed == 0) {
        return 0;
    }

    if (!Record::Read(&_ssid, sizeof(_ssid), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_ssid) == 0 || strlen(_ssid) >= sizeof(_ssid)) {
        return 0;
    }
    SetSsid(_ssid);
    return readed;
}

TvElementType WiFiBinding::GetElementType() {
    return TvElementType::et_WiFiBinding;
}

WiFiBinding *WiFiBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiBinding:
            return static_cast<WiFiBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiBinding::SelectPriorSymbol(char *symbol) {
    if (*symbol > '!' && *symbol <= '~') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '~';
    }
}

void WiFiBinding::SelectNextSymbol(char *symbol) {
    if (*symbol >= '!' && *symbol < '~') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '!';
    }
}

void WiFiBinding::SelectPrior() {
    log_i(TAG_WiFiBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            CommonWiFiBinding::SelectPrior();
            break;

        default:
            SelectPriorSymbol(
                &ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char]);
            break;
    }
}

void WiFiBinding::SelectNext() {
    log_i(TAG_WiFiBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            CommonWiFiBinding::SelectNext();
            break;

        default:
            SelectNextSymbol(
                &ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char]);
            break;
    }
}

void WiFiBinding::PageUp() {
    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            this->SelectPrior();
            break;

        default:
            ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                place_new_char;
            break;
    }
}

void WiFiBinding::PageDown() {
    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            this->SelectNext();
            break;

        default:
            ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                place_new_char;
            break;
    }
}

bool WiFiBinding::IsLastSsidChar() {
    char ch = ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char];
    return ch == 0 || ch == place_new_char;
}

bool WiFiBinding::ChangeSsid() {
    if (editing_property_id == WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char
        || IsLastSsidChar()) {
        return false;
    }

    editing_property_id++;
    if (IsLastSsidChar()) {
        ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
            place_new_char;
    }
    return true;
}

void WiFiBinding::Change() {
    log_i(TAG_WiFiBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            CommonWiFiBinding::Change();
            break;
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;
            if (IsLastSsidChar()) {
                ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                    place_new_char;
            }
            break;

        default:
            if (editing_property_id <= WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                if (!ChangeSsid()) {
                    editing_property_id = WiFiBinding::EditingPropertyId::wbepi_None;
                    EndEditing();
                }
            }
            break;
    }
}

void WiFiBinding::Option() {
    log_i(TAG_WiFiBinding, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            break;

        default:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_None;
            EndEditing();
            break;
    }
}

void WiFiBinding::EndEditing() {
    ssid_size = 0;
    while (ssid_size < sizeof(ssid) && ssid[ssid_size] != 0 && ssid[ssid_size] != place_new_char) {
        ssid_size++;
    }
    ssid[ssid_size] = 0;
    EditableElement::EndEditing();
}

const char *WiFiBinding::GetSsid() {
    return ssid;
}

void WiFiBinding::SetSsid(const char *ssid) {
    strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
    this->ssid[sizeof(this->ssid) - 1] = 0;
    ssid_size = strlen(this->ssid);
}