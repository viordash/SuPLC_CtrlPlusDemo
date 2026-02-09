#pragma once

#include "LogicProgram/Bindings/WiFiBinding.h"
#include <stdint.h>
#include <unistd.h>

class WiFiApBinding : public WiFiBinding {
  protected:
    static const uint8_t max_password_size = 16;
    char password[max_password_size + 1];
    uint8_t password_size;

    static const char mac_wild_char = '*';
    static const uint8_t mac_size = 12;
    char mac[mac_size + 1];

    const AllowedIO GetAllowedInputs() override final;
    void RenderEditedPassword(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y);
    bool IsLastPasswordChar();
    bool ChangePassword();

    void SelectPriorMacSymbol(char *symbol);
    void SelectNextMacSymbol(char *symbol);
    void RenderEditedMac(FrameBuffer *fb, uint16_t left, uint16_t right, uint16_t y);

  public:
    typedef enum { //
        wbepi_None = WiFiBinding::EditingPropertyId::wbepi_None,
        wbepi_ConfigureIOAdr = WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
        wbepi_Ssid_First_Char = WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
        wbepi_Ssid_Last_Char = WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char,
        wbepi_Password_First_Char,
        wbepi_Password_Last_Char = wbepi_Password_First_Char + max_password_size - 1,
        wbepi_Mac_First_Char,
        wbepi_Mac_Last_Char = wbepi_Mac_First_Char + mac_size - 1
    } EditingPropertyId;

    explicit WiFiApBinding();
    explicit WiFiApBinding(const MapIO io_adr,
                           const char *ssid,
                           const char *password,
                           const char *mac);
    virtual ~WiFiApBinding();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;
    TvElementType GetElementType() override;

    static WiFiApBinding *TryToCast(LogicElement *logic_element);

    void EndEditing() override;
    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;

    const char *GetPassword();
    void SetPassword(const char *password);
    const char *GetMac();
    void SetMac(const char *mac);

    static bool ClientMacMatches(const char *mask, const uint8_t mac[6]);
};
