#include "LogicProgram/Bindings/WiFiApBinding.h"
#include "DeviceIO.h"
#include "LogicProgram/Serializer/Record.h"
#include "Utils/Converter.h"
#include "board.h"
#include "os.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiApBinding = "WiFiApBinding";

WiFiApBinding::WiFiApBinding() : WiFiBinding() {
    password[0] = 0;
    password_size = 0;
    mac[0] = 0;
}

WiFiApBinding::WiFiApBinding(const MapIO io_adr,
                             const char *ssid,
                             const char *password,
                             const char *mac)
    : WiFiBinding(io_adr, ssid) {
    SetPassword(password);
    SetMac(mac);
}

WiFiApBinding::~WiFiApBinding() {
}

bool WiFiApBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        DeviceIO::BindVariableToSecureWiFi(GetIoAdr(), GetSsid(), GetPassword(), GetMac());
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        DeviceIO::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        log_d(TAG_WiFiApBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR void
WiFiApBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::mutex> lock(lock_mutex);

    Point draw_point = *start_point;
    CommonWiFiBinding::Render(fb, prev_elem_state, &draw_point);

    draw_point.x += body_font.GetRightLeftMargin();
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr: {
            uint16_t text_y = draw_point.y - body_font.GetHeight() / 2;
            ASSERT(body_font.DrawText(fb, draw_point.x, text_y, "AP CLNT") > 0);
            break;
        }

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                RenderEditedSsid(fb,
                                 draw_point.x,
                                 (start_point->x + WifiBindingLeftPadding + WifiBindingWidth)
                                     - value_font.GetRightLeftMargin(),
                                 draw_point.y);

            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                RenderEditedPassword(fb,
                                     draw_point.x,
                                     (start_point->x + WifiBindingLeftPadding + WifiBindingWidth)
                                         - value_font.GetRightLeftMargin(),
                                     draw_point.y);
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                RenderEditedMac(fb,
                                draw_point.x,
                                (start_point->x + WifiBindingLeftPadding + WifiBindingWidth)
                                    - value_font.GetRightLeftMargin(),
                                draw_point.y);
            }
            break;
    }

    start_point->x += WifiBindingLeftPadding + WifiBindingWidth;
}

void WiFiApBinding::RenderEditedPassword(FrameBuffer *fb,
                                         uint16_t left,
                                         uint16_t right,
                                         uint16_t y) {
    char editing_value[max_password_size + 1];
    int char_pos =
        editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;

    strncpy(editing_value, password, sizeof(editing_value) - 1);
    editing_value[sizeof(editing_value) - 1] = 0;

    if (Blinking_50(fb)) {
        editing_value[char_pos] = ' ';
    }

    uint16_t top = y - (WifiBindingHeight / 2);
    uint16_t bottom = top + WifiBindingHeight - 1;
    ASSERT(caption_font.DrawText(fb,
                                 left + caption_font.GetRightLeftMargin(),
                                 top + caption_font.GetTopMargin(),
                                 "PASSWORD:")
           <= 0);
    ASSERT(value_font.DrawLimitText(fb,
                                    left + value_font.GetRightLeftMargin(),
                                    right,
                                    bottom - value_font.GetHeight(),
                                    editing_value,
                                    &editing_value[char_pos])
           > 0);
}

void WiFiApBinding::RenderEditedMac(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y) {
    char editing_value[mac_size + 1];
    int char_pos = editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char;

    strncpy(editing_value, mac, sizeof(editing_value) - 1);
    editing_value[sizeof(editing_value) - 1] = 0;

    if (Blinking_50(fb)) {
        editing_value[char_pos] = ' ';
    }

    uint16_t top = y - (WifiBindingHeight / 2);
    uint16_t bottom = top + WifiBindingHeight - 1;
    ASSERT(caption_font.DrawText(fb,
                                 left + caption_font.GetRightLeftMargin(),
                                 top + caption_font.GetTopMargin(),
                                 "MAC:")
           <= 0);
    ASSERT(value_font.DrawLimitText(fb,
                                    left + value_font.GetRightLeftMargin(),
                                    right,
                                    bottom - value_font.GetHeight(),
                                    editing_value,
                                    &editing_value[char_pos])
           > 0);
}

size_t WiFiApBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = WiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    if (!Record::Write(&password, sizeof(password), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&mac, sizeof(mac), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiApBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = WiFiBinding::Deserialize(buffer, buffer_size);
    if (readed == 0) {
        return 0;
    }
    char _password[sizeof(password)];
    if (!Record::Read(&_password, sizeof(_password), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_password) == 0 || strlen(_password) >= sizeof(_password)) {
        return 0;
    }
    SetPassword(_password);

    char _mac[sizeof(mac)];
    if (!Record::Read(&_mac, sizeof(_mac), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_mac) == 0 || strlen(_mac) != sizeof(_mac) - 1) {
        return 0;
    }
    SetMac(_mac);
    return readed;
}

TvElementType WiFiApBinding::GetElementType() {
    return TvElementType::et_WiFiApBinding;
}

WiFiApBinding *WiFiApBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiApBinding:
            return static_cast<WiFiApBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiApBinding::SelectPriorMacSymbol(char *symbol) {
    if ((*symbol > '0' && *symbol <= '9') || (*symbol > 'A' && *symbol <= 'F')) {
        *symbol = *symbol - 1;
    } else if (*symbol == mac_wild_char) {
        *symbol = 'F';
    } else if (*symbol == 'A') {
        *symbol = '9';
    } else {
        *symbol = mac_wild_char;
    }
}

void WiFiApBinding::SelectNextMacSymbol(char *symbol) {
    if ((*symbol >= '0' && *symbol < '9') || (*symbol >= 'A' && *symbol < 'F')) {
        *symbol = *symbol + 1;
    } else if (*symbol == '9') {
        *symbol = 'A';
    } else if (*symbol == mac_wild_char) {
        *symbol = '0';
    } else {
        *symbol = mac_wild_char;
    }
}

void WiFiApBinding::SelectPrior() {
    log_i(TAG_WiFiApBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::SelectPrior();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::SelectPrior();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                SelectPriorSymbol(
                    &password[editing_property_id
                              - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char]);
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                SelectPriorMacSymbol(
                    &mac[editing_property_id
                         - WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char]);
            }

            break;
    }
}

void WiFiApBinding::SelectNext() {
    log_i(TAG_WiFiApBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::SelectNext();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::SelectNext();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                SelectNextSymbol(
                    &password[editing_property_id
                              - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char]);
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                SelectNextMacSymbol(&mac[editing_property_id
                                         - WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char]);
            }
            break;
    }
}

void WiFiApBinding::PageUp() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::PageUp();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::PageUp();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                password[editing_property_id
                         - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                    place_new_char;
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                mac[editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char] =
                    mac_wild_char;
            }
            break;
    }
}

void WiFiApBinding::PageDown() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::PageDown();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::PageDown();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                password[editing_property_id
                         - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                    place_new_char;
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                mac[editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char] =
                    mac_wild_char;
            }
            break;
    }
}

bool WiFiApBinding::IsLastPasswordChar() {
    char ch =
        password[editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char];
    return ch == 0 || ch == place_new_char;
}

bool WiFiApBinding::ChangePassword() {
    if (editing_property_id == WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char
        || IsLastPasswordChar()) {
        return false;
    }

    editing_property_id++;
    if (IsLastPasswordChar()) {
        password[editing_property_id
                 - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] = place_new_char;
    }
    return true;
}

void WiFiApBinding::Change() {
    log_i(TAG_WiFiApBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            WiFiBinding::Change();
            break;

        default:
            if (editing_property_id <= WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                if (!ChangeSsid()) {
                    editing_property_id =
                        WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;
                    if (IsLastPasswordChar()) {
                        password[editing_property_id
                                 - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                            place_new_char;
                    }
                }
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                if (!ChangePassword()) {
                    editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char;
                }
            } else if (editing_property_id
                       < WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                editing_property_id++;
            } else {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_None;
                EndEditing();
            }
            break;
    }
}

void WiFiApBinding::Option() {
    log_i(TAG_WiFiApBinding, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            WiFiBinding::Option();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;
                if (IsLastPasswordChar()) {
                    password[editing_property_id
                             - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                        place_new_char;
                }
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char;
                if (IsLastPasswordChar()) {
                    password[editing_property_id
                             - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                        place_new_char;
                }

            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_None;
                EndEditing();
            }
            break;
    }
}

void WiFiApBinding::EndEditing() {
    password_size = 0;
    while (password_size < sizeof(password) && password[password_size] != 0
           && password[password_size] != place_new_char) {
        password_size++;
    }
    password[password_size] = 0;
    WiFiBinding::EndEditing();
}

const char *WiFiApBinding::GetPassword() {
    return password;
}

void WiFiApBinding::SetPassword(const char *password) {
    strncpy(this->password, password, sizeof(this->password) - 1);
    this->password[sizeof(this->password) - 1] = 0;
    password_size = strlen(this->password);
}

const char *WiFiApBinding::GetMac() {
    return mac;
}

void WiFiApBinding::SetMac(const char *mac) {
    size_t s_len = strlen(mac);
    for (size_t i = 0; i < sizeof(this->mac) - 1; i++) {
        char ch = i < s_len ? toupper(mac[i]) : mac_wild_char;
        this->mac[i] = ch;
    }
    this->mac[sizeof(this->mac) - 1] = 0;
}

bool WiFiApBinding::ClientMacMatches(const char *mask, const uint8_t mac[6]) {
    int i = 0;
    while (i < mac_size) {
        uint8_t mac_b = mac[i / 2];
        char mask_h = mask[i++];
        if (mask_h == 0) {
            return false;
        }
        char mask_l = mask[i++];
        if (mask_l == 0) {
            return false;
        }

        uint8_t dec_h = (mac_b >> 4) & 0x0F;
        uint8_t dec_l = mac_b & 0x0F;

        if (mask_h != mac_wild_char && hexchar_to_dec(mask_h) != dec_h) {
            return false;
        }
        if (mask_l != mac_wild_char && hexchar_to_dec(mask_l) != dec_l) {
            return false;
        }
    }
    return true;
}