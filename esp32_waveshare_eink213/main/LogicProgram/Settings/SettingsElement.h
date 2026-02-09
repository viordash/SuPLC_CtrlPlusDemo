#pragma once

#include "Display/Font.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class SettingsElement : public LogicElement {
  public:
    typedef enum { //
        t_wifi_station_settings_ssid = 0,
        t_wifi_station_settings_password,
        t_wifi_station_settings_connect_max_retry_count,
        t_wifi_station_settings_reconnect_delay_ms,
        t_wifi_station_settings_scan_station_rssi_period_ms,
        t_wifi_station_settings_max_rssi,
        t_wifi_station_settings_min_rssi,
        t_wifi_station_settings_min_worktime_ms,

        t_wifi_scanner_settings_per_channel_scan_time_ms,
        t_wifi_scanner_settings_max_rssi,
        t_wifi_scanner_settings_min_rssi,

        t_wifi_access_point_settings_generation_time_ms,
        t_wifi_access_point_settings_ssid_hidden,

        t_current_date,
        t_current_time,

        t_datetime_sntp_server_primary,
        t_datetime_sntp_server_secondary,
        t_datetime_timezone,
        t_adc_scan_period_ms,
    } Discriminator;

  protected:
    static const char place_new_char = 0x02;
    static const uint8_t value_size = 64;
    char value[value_size + 1];

    FontSmall caption_font;
    FontLarge value_font;

    Discriminator discriminator;
    bool ValidateDiscriminator(Discriminator discriminator);

    void RenderName(FrameBuffer *fb, uint16_t x, uint16_t y);
    void RenderEditedValue(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y);
    bool IsLastValueChar();
    bool ChangeValue();
    void ReadValue(char *string_buffer, bool friendly_format);
    void SelectPriorStringSymbol(char *symbol);
    void SelectNextStringSymbol(char *symbol);
    void SelectPriorNumberSymbol(char *symbol, char extra);
    void SelectNextNumberSymbol(char *symbol, char extra);
    void SelectBoolSymbol(char *symbol);
    void SelectPriorSymbol(char *symbol, bool first);
    void SelectNextSymbol(char *symbol, bool first);
    void WriteString(char *dest, size_t dest_size);
    bool ParseDateValue();
    bool ParseTimeValue();
    const Bitmap *GetCurrentBitmap();

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_SelectDiscriminator,
        cwbepi_Value_First_Char,
        cwbepi_Value_Last_Char = cwbepi_Value_First_Char + value_size - 1
    } EditingPropertyId;

    explicit SettingsElement();
    explicit SettingsElement(const Discriminator discriminator);
    virtual ~SettingsElement();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
    void EndEditing() override;

    TvElementType GetElementType() override final;
    static SettingsElement *TryToCast(LogicElement *logic_element);
};
