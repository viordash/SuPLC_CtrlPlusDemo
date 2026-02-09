#include "LogicProgram/Settings/SettingsElement.h"
#include "Datetime/Datetime.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "icons/settings.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CurrentSettings::device_settings settings;

static const char *TAG_SettingsElement = "SettingsElement";

SettingsElement::SettingsElement() : LogicElement() {
    value[0] = 0;
    discriminator = Discriminator::t_wifi_station_settings_ssid;
}

SettingsElement::SettingsElement(const Discriminator discriminator) : SettingsElement() {
    this->discriminator = discriminator;
}

SettingsElement::~SettingsElement() {
}

bool SettingsElement::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    state = prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR void
SettingsElement::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    if (prev_elem_state == LogicItemState::lisActive) {
        ASSERT(draw_active_network(fb, start_point->x, start_point->y, SettingsLeftPadding));
    } else {
        ASSERT(
            draw_passive_network(fb, start_point->x, start_point->y, SettingsLeftPadding, false));
    }

    start_point->x += SettingsLeftPadding;

    Point top_left = { start_point->x, (uint16_t)(start_point->y - (SettingsHeight / 2)) };
    Point bottom_left = { start_point->x, (uint16_t)(top_left.y + SettingsHeight) };
    Point top_right = { (uint16_t)(start_point->x + SettingsWidth), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (SettingsElement::EditingPropertyId)editing_property_id
                                     == SettingsElement::EditingPropertyId::cwbepi_None
                              && Blinking_50(fb);
    if (!blink_body_on_editing) {
        ASSERT(draw_rectangle(fb,
                              top_left.x,
                              top_left.y,
                              SettingsWidth + 1,
                              SettingsHeight + 1,
                              state != LogicItemState::lisActive));

        draw_bitmap(fb,
                    start_point->x,
                    start_point->y - (GetCurrentBitmap()->size.height / 2),
                    GetCurrentBitmap());
    }

    start_point->x += GetCurrentBitmap()->size.width;

    bool blink_name = editable_state == EditableElement::ElementState::des_Editing
                   && (SettingsElement::EditingPropertyId)editing_property_id
                          == SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator
                   && Blinking_50(fb);

    if (!blink_name) {
        RenderName(fb, start_point->x, top_left.y);
    }

    bool show_edit_value = editable_state == EditableElement::ElementState::des_Editing
                        && (SettingsElement::EditingPropertyId)editing_property_id
                               != SettingsElement::EditingPropertyId::cwbepi_None
                        && (SettingsElement::EditingPropertyId)editing_property_id
                               != SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator;

    if (show_edit_value) {
        RenderEditedValue(fb,
                          start_point->x + value_font.GetRightLeftMargin(),
                          top_right.x - value_font.GetRightLeftMargin(),
                          bottom_left.y - 1 - value_font.GetHeight());
    } else {
        char display_value[value_size + 1];
        ReadValue(display_value, true);
        if (display_value[0] != 0) {
            ASSERT(value_font.DrawText(fb,
                                       start_point->x + value_font.GetRightLeftMargin(),
                                       top_right.x - value_font.GetRightLeftMargin(),
                                       bottom_left.y - 1 - value_font.GetHeight(),
                                       display_value,
                                       FontBase::Align::Right)
                   > 0);
        }
    }

    start_point->x = top_right.x;
    top_right.y += (SettingsHeight / 2);
    EditableElement::Render(fb, &top_right);
}

void SettingsElement::RenderName(FrameBuffer *fb, uint16_t x, uint16_t y) {
    const char *name;

    switch (discriminator) {
        case t_wifi_station_settings_ssid: {
            name = "STA: ssid";
            break;
        }
        case t_wifi_station_settings_password: {
            name = "STA: password";
            break;
        }
        case t_wifi_station_settings_connect_max_retry_count:
            name = "STA: retry count max";
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            name = "STA: reconn pause,mS";
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            name = "STA: rssi period, mS";
            break;
        case t_wifi_station_settings_max_rssi:
            name = "STA: max rssi, dBm";
            break;
        case t_wifi_station_settings_min_rssi:
            name = "STA: min rssi, dBm";
            break;
        case t_wifi_station_settings_min_worktime_ms:
            name = "STA: worktime, ms";
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            name = "SCAN: scan time, mS";
            break;
        case t_wifi_scanner_settings_max_rssi:
            name = "SCAN: max rssi, dBm";
            break;
        case t_wifi_scanner_settings_min_rssi:
            name = "SCAN: min rssi, dBm";
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            name = "AP: br-cast time, mS";
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            name = "AP: hidden ssid";
            break;
        case t_current_date:
            name = "Date YYYY-MM-DD";
            break;
        case t_current_time:
            name = "Time hh:mm:ss";
            break;
        case t_datetime_sntp_server_primary:
            name = "Primary SNTP server";
            break;
        case t_datetime_sntp_server_secondary:
            name = "Second. SNTP server";
            break;
        case t_datetime_timezone:
            name = "Timezone";
            break;
        case t_adc_scan_period_ms:
            name = "ADC: scan period, mS";
            break;
        default:
            return;
    }

    ASSERT(caption_font.DrawText(fb,
                                 x + caption_font.GetRightLeftMargin(),
                                 y + caption_font.GetTopMargin(),
                                 name)
           > 0);
}

void SettingsElement::RenderEditedValue(FrameBuffer *fb,
                                        uint16_t left,
                                        uint16_t right,
                                        uint16_t y) {
    char editing_value[value_size + 1];
    int char_pos =
        editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;

    strncpy(editing_value, value, sizeof(editing_value) - 1);
    editing_value[sizeof(editing_value) - 1] = 0;

    if (Blinking_50(fb)) {
        editing_value[char_pos] = ' ';
    }

    ASSERT(value_font.DrawLimitText(fb, left, right, y, editing_value, &editing_value[char_pos])
           > 0);
}

size_t SettingsElement::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();

    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&discriminator, sizeof(discriminator), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t SettingsElement::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    Discriminator _discriminator;
    if (!Record::Read(&_discriminator, sizeof(_discriminator), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateDiscriminator(_discriminator)) {
        return 0;
    }

    this->discriminator = _discriminator;
    return readed;
}

bool SettingsElement::ValidateDiscriminator(Discriminator discriminator) {
    switch (discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
        case t_wifi_station_settings_connect_max_retry_count:
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
        case t_wifi_station_settings_min_worktime_ms:
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
        case t_wifi_access_point_settings_generation_time_ms:
        case t_wifi_access_point_settings_ssid_hidden:
        case t_current_date:
        case t_current_time:
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
        case t_adc_scan_period_ms:
            return true;

        default:
            return false;
    }
}

bool SettingsElement::IsLastValueChar() {
    char ch =
        value[editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char];
    return ch == 0 || ch == place_new_char;
}

bool SettingsElement::ChangeValue() {
    if (editing_property_id == SettingsElement::EditingPropertyId::cwbepi_Value_Last_Char
        || IsLastValueChar()) {
        return false;
    }

    editing_property_id++;
    if (IsLastValueChar()) {
        value[editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] =
            place_new_char;
    }
    return true;
}

void SettingsElement::ReadValue(char *string_buffer, bool friendly_format) {
    CurrentSettings::device_settings curr_settings;
    SAFETY_SETTINGS({ curr_settings = settings; });

    switch (discriminator) {
        case t_wifi_station_settings_ssid: {
            const size_t max_len = sizeof(curr_settings.wifi_station.ssid);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.wifi_station.ssid, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_wifi_station_settings_password: {
            const size_t max_len = sizeof(curr_settings.wifi_station.password);
            static_assert(value_size + 1 > max_len);
            if (friendly_format) {
                strcpy(string_buffer, "********");
            } else {
                strncpy(string_buffer, curr_settings.wifi_station.password, max_len);
                string_buffer[max_len] = 0;
            }
            break;
        }
        case t_wifi_station_settings_connect_max_retry_count:
            if (friendly_format
                && curr_settings.wifi_station.connect_max_retry_count == INFINITY_CONNECT_RETRY) {
                strcpy(string_buffer, "infinity");
            } else {
                sprintf(string_buffer,
                        "%d",
                        (int)curr_settings.wifi_station.connect_max_retry_count);
            }
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_station.reconnect_delay_ms);
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_station.scan_station_rssi_period_ms);
            break;
        case t_wifi_station_settings_max_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_station.max_rssi);
            break;
        case t_wifi_station_settings_min_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_station.min_rssi);
            break;
        case t_wifi_station_settings_min_worktime_ms:
            sprintf(string_buffer, "%u", (unsigned int)curr_settings.wifi_station.min_worktime_ms);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_scanner.per_channel_scan_time_ms);
            break;
        case t_wifi_scanner_settings_max_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_scanner.max_rssi);
            break;
        case t_wifi_scanner_settings_min_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_scanner.min_rssi);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_access_point.generation_time_ms);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            if (friendly_format) {
                strcpy(string_buffer,
                       curr_settings.wifi_access_point.ssid_hidden ? "true" : "false");
            } else {
                sprintf(string_buffer,
                        "%u",
                        (unsigned int)curr_settings.wifi_access_point.ssid_hidden);
            }
            break;
        case t_current_date: {
            if (state == LogicItemState::lisActive
                || (SettingsElement::EditingPropertyId)editing_property_id
                       != SettingsElement::EditingPropertyId::cwbepi_None) {
                Datetime dt;
                Controller::GetSystemDatetime(&dt);
                sprintf(string_buffer, "%04d-%02d-%02d", (int)dt.year, (int)dt.month, (int)dt.day);
            } else {
                sprintf(string_buffer, "  -  -  ");
            }
            break;
        }
        case t_current_time: {
            if (state == LogicItemState::lisActive
                || (SettingsElement::EditingPropertyId)editing_property_id
                       != SettingsElement::EditingPropertyId::cwbepi_None) {
                Datetime dt;
                Controller::GetSystemDatetime(&dt);
                sprintf(string_buffer,
                        "%02d:%02d:%02d",
                        (int)dt.hour,
                        (int)dt.minute,
                        (int)dt.second);
            } else {
                sprintf(string_buffer, "  :  :  ");
            }
            break;
        }
        case t_datetime_sntp_server_primary: {
            const size_t max_len = sizeof(curr_settings.datetime.sntp_server_primary);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.sntp_server_primary, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_datetime_sntp_server_secondary: {
            const size_t max_len = sizeof(curr_settings.datetime.sntp_server_secondary);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.sntp_server_secondary, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_datetime_timezone: {
            const size_t max_len = sizeof(curr_settings.datetime.timezone);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.timezone, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_adc_scan_period_ms:
            sprintf(string_buffer, "%u", (unsigned int)curr_settings.adc.scan_period_ms);
            break;

        default:
            break;
    }
}

void SettingsElement::SelectPriorStringSymbol(char *symbol) {
    if (*symbol > '!' && *symbol <= '~') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '~';
    }
}

void SettingsElement::SelectNextStringSymbol(char *symbol) {
    if (*symbol >= '!' && *symbol < '~') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '!';
    }
}

void SettingsElement::SelectPriorNumberSymbol(char *symbol, char extra) {
    if (extra != 0 && *symbol == '0') {
        *symbol = extra;
    } else if (*symbol > '0' && *symbol <= '9') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '9';
    }
}
void SettingsElement::SelectNextNumberSymbol(char *symbol, char extra) {
    if (extra != 0 && *symbol == '9') {
        *symbol = extra;
    } else if (*symbol >= '0' && *symbol < '9') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '0';
    }
}

void SettingsElement::SelectBoolSymbol(char *symbol) {
    if (*symbol != '0') {
        *symbol = '0';
    } else {
        *symbol = '1';
    }
}

void SettingsElement::SelectPriorSymbol(char *symbol, bool first) {
    switch (discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
            SelectPriorStringSymbol(symbol);
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_min_worktime_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            SelectBoolSymbol(symbol);
            break;
        case t_current_date:
            SelectPriorNumberSymbol(symbol, '-');
            break;
        case t_current_time:
            SelectPriorNumberSymbol(symbol, ':');
            break;
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
            SelectPriorStringSymbol(symbol);
            break;
        case t_adc_scan_period_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;

        default:
            break;
    }
}

void SettingsElement::SelectNextSymbol(char *symbol, bool first) {
    switch (discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
            SelectNextStringSymbol(symbol);
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_min_worktime_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            SelectBoolSymbol(symbol);
            break;
        case t_current_date:
            SelectNextNumberSymbol(symbol, '-');
            break;
        case t_current_time:
            SelectNextNumberSymbol(symbol, ':');
            break;
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
            SelectNextStringSymbol(symbol);
            break;
        case t_adc_scan_period_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        default:
            break;
    }
}

void SettingsElement::SelectPrior() {
    log_i(TAG_SettingsElement, "SelectPrior");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator: {
            auto _discriminator = (Discriminator)(discriminator - 1);
            if (!ValidateDiscriminator(_discriminator)) {
                _discriminator = Discriminator::t_adc_scan_period_ms;
            }
            discriminator = _discriminator;
            break;
        }

        default: {
            int pos =
                editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            SelectPriorSymbol(&value[pos], pos == 0);
            break;
        }
    }
}

void SettingsElement::SelectNext() {
    log_i(TAG_SettingsElement, "SelectNext");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator: {
            auto _discriminator = (Discriminator)(discriminator + 1);
            if (!ValidateDiscriminator(_discriminator)) {
                _discriminator = Discriminator::t_wifi_station_settings_ssid;
            }
            discriminator = _discriminator;
            break;
        }

        default: {
            int pos =
                editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            SelectNextSymbol(&value[pos], pos == 0);
            break;
        }
    }
}

void SettingsElement::PageUp() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
        case SettingsElement::EditingPropertyId::cwbepi_Value_First_Char:
            this->SelectPrior();
            break;

        default:
            value[editing_property_id
                  - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] = place_new_char;
            break;
    }
}

void SettingsElement::PageDown() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
        case SettingsElement::EditingPropertyId::cwbepi_Value_First_Char:
            this->SelectNext();
            break;

        default:
            value[editing_property_id
                  - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] = place_new_char;
            break;
    }
}

void SettingsElement::Change() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator;
            break;

        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            memset(value, 0, sizeof(value));
            ReadValue(value, false);
            if (IsLastValueChar()) {
                value[editing_property_id
                      - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] =
                    place_new_char;
            }
            break;

        default:
            if (editing_property_id <= SettingsElement::EditingPropertyId::cwbepi_Value_Last_Char) {
                if (!ChangeValue()) {
                    editing_property_id = SettingsElement::EditingPropertyId::cwbepi_None;
                    EndEditing();
                }
            }
            break;
    }
}
void SettingsElement::Option() {
    log_i(TAG_SettingsElement, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
            break;

        default:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_None;
            EndEditing();
            break;
    }
}

void SettingsElement::WriteString(char *dest, size_t dest_size) {
    size_t size = 0;
    while (size < dest_size && value[size] != 0 && value[size] != place_new_char) {
        *dest++ = value[size];
        size++;
    }
    if (size < dest_size) {
        *dest = 0;
    }
}

void SettingsElement::EndEditing() {
    CurrentSettings::device_settings curr_settings;
    SAFETY_SETTINGS({ curr_settings = settings; });

    switch (discriminator) {
        case t_wifi_station_settings_ssid:
            WriteString(curr_settings.wifi_station.ssid, sizeof(curr_settings.wifi_station.ssid));
            break;
        case t_wifi_station_settings_password:
            WriteString(curr_settings.wifi_station.password,
                        sizeof(curr_settings.wifi_station.password));
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            curr_settings.wifi_station.connect_max_retry_count = atoi(value);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            curr_settings.wifi_station.reconnect_delay_ms = atol(value);
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            curr_settings.wifi_station.scan_station_rssi_period_ms = atol(value);
            break;
        case t_wifi_station_settings_max_rssi:
            curr_settings.wifi_station.max_rssi = atoi(value);
            break;
        case t_wifi_station_settings_min_rssi:
            curr_settings.wifi_station.min_rssi = atoi(value);
            break;
        case t_wifi_station_settings_min_worktime_ms:
            curr_settings.wifi_station.min_worktime_ms = atol(value);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            curr_settings.wifi_scanner.per_channel_scan_time_ms = atol(value);
            break;
        case t_wifi_scanner_settings_max_rssi:
            curr_settings.wifi_scanner.max_rssi = atoi(value);
            break;
        case t_wifi_scanner_settings_min_rssi:
            curr_settings.wifi_scanner.min_rssi = atoi(value);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            curr_settings.wifi_access_point.generation_time_ms = atol(value);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            curr_settings.wifi_access_point.ssid_hidden = atol(value) != 0;
            break;
        case t_current_date:
            if (!ParseDateValue()) {
                return;
            }
            break;
        case t_current_time:
            if (!ParseTimeValue()) {
                return;
            }
            break;
        case t_datetime_sntp_server_primary:
            WriteString(curr_settings.datetime.sntp_server_primary,
                        sizeof(curr_settings.datetime.sntp_server_primary));
            break;
        case t_datetime_sntp_server_secondary:
            WriteString(curr_settings.datetime.sntp_server_secondary,
                        sizeof(curr_settings.datetime.sntp_server_secondary));
            break;
        case t_datetime_timezone:
            WriteString(curr_settings.datetime.timezone, sizeof(curr_settings.datetime.timezone));
            break;
        case t_adc_scan_period_ms:
            curr_settings.adc.scan_period_ms = atol(value);
            break;
        default:
            break;
    }

    if (discriminator != t_current_date && discriminator != t_current_time) {
        if (validate_settings(&curr_settings)) {
            SAFETY_SETTINGS(              //
                settings = curr_settings; //
                store_settings();         //
            );

            switch (discriminator) {
                case t_datetime_sntp_server_primary:
                case t_datetime_sntp_server_secondary:
                case t_datetime_timezone:
                    Controller::RestartSntp();
                    break;
                default:
                    break;
            }

        } else {
            log_e(TAG_SettingsElement, "Settings changing has some error");
        }
    }

    log_i(TAG_SettingsElement, "Settings changed successfully");
    EditableElement::EndEditing();
}

bool SettingsElement::ParseDateValue() {
    Datetime dt;
    Controller::GetSystemDatetime(&dt);
    const int date_elements_count = 3;
    int count = sscanf(value, "%4d-%2d-%2d", (int *)&dt.year, (int *)&dt.month, (int *)&dt.day);
    if (count != date_elements_count) {
        log_w(TAG_SettingsElement, "ReadDateValue, invalid count:%d", count);
        return false;
    }
    return Controller::ManualSetSystemDatetime(&dt);
}

bool SettingsElement::ParseTimeValue() {
    Datetime dt;
    Controller::GetSystemDatetime(&dt);
    const int date_elements_count = 3;
    int count = sscanf(value, "%2d:%2d:%2d", (int *)&dt.hour, (int *)&dt.minute, (int *)&dt.second);
    if (count != date_elements_count) {
        log_w(TAG_SettingsElement, "ReadTimeValue, invalid count:%d", count);
        return false;
    }
    return Controller::ManualSetSystemDatetime(&dt);
}

TvElementType SettingsElement::GetElementType() {
    return TvElementType::et_Settings;
}

SettingsElement *SettingsElement::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Settings:
            return static_cast<SettingsElement *>(logic_element);

        default:
            return NULL;
    }
}

const Bitmap *SettingsElement::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}