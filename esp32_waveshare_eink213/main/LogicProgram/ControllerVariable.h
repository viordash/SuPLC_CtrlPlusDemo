#pragma once

#include "Datetime/DatetimeService.h"
#include "LogicProgram/Bindings/DatetimePart.h"
#include "LogicProgram/ControllerBaseInputOutput.h"
#include "WiFi/WiFiService.h"
#include <stdint.h>
#include <unistd.h>

class ControllerVariable : public ControllerBaseInputOutput {
  protected:
    WiFiService *wifi_service;
    const char *ssid;
    const char *password;
    const char *mac;

    DatetimeService *datetime_service;
    DatetimePart datetime_part;

  public:
    explicit ControllerVariable(const char *name);
    void FetchValue() override;
    void CommitChanges() override;
    void BindToSecureWiFi(WiFiService *wifi_service,
                          const char *ssid,
                          const char *password,
                          const char *mac);
    void BindToInsecureWiFi(WiFiService *wifi_service, const char *ssid);
    void BindToStaWiFi(WiFiService *wifi_service);
    void Unbind();
    bool BindedToWiFi();

    void BindToDateTime(DatetimeService *datetime_service, DatetimePart datetime_part);

    bool BindedToDateTime();

    void CancelReadingProcess() override;
};
