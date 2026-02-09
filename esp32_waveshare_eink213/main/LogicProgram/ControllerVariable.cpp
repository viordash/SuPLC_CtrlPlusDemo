#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/Bindings/DateTimeBinding.h"
#include "LogicProgram/LogicElement.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerVariable = "ControllerVariable";

ControllerVariable::ControllerVariable(const char *name) : ControllerBaseInputOutput(name) {
    this->wifi_service = NULL;
    ssid = NULL;
    password = NULL;
    mac = NULL;
    this->datetime_service = NULL;
    this->datetime_part = DatetimePart::t_second;
}

void ControllerVariable::FetchValue() {
    if (!required_reading) {
        return;
    }
    required_reading = false;

    if (BindedToWiFi()) {
        bool insecure_scan = ssid != NULL && password == NULL && mac == NULL;
        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;

        if (insecure_scan) {
            UpdateValue(wifi_service->Scan(ssid));
        } else if (secure_ap) {
            UpdateValue(wifi_service->AccessPoint(ssid, password, mac));
        } else {
            UpdateValue(wifi_service->ConnectToStation());
        }
    } else if (BindedToDateTime()) {
        switch (datetime_part) {
            case DatetimePart::t_second:
                UpdateValue(datetime_service->GetCurrentSecond());
                break;
            case DatetimePart::t_minute:
                UpdateValue(datetime_service->GetCurrentMinute());
                break;
            case DatetimePart::t_hour:
                UpdateValue(datetime_service->GetCurrentHour());
                break;
            case DatetimePart::t_day:
                UpdateValue(datetime_service->GetCurrentDay());
                break;
            case DatetimePart::t_weekday:
                UpdateValue(datetime_service->GetCurrentWeekday());
                break;
            case DatetimePart::t_month:
                UpdateValue(datetime_service->GetCurrentMonth());
                break;
            case DatetimePart::t_year:
                UpdateValue(datetime_service->GetCurrentYear());
                break;
        }

    } else {
        UpdateValue(out_value);
    }
}

void ControllerVariable::CommitChanges() {
    if (!required_writing) {
        return;
    }
    required_writing = false;
    UpdateValue(out_value);
    if (BindedToWiFi()) {
        bool wifi_sta_client = ssid == NULL;
        if (wifi_sta_client) {
            return;
        }

        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;
        if (secure_ap) {
            return;
        }

        if (out_value != LogicElement::MinValue) {
            wifi_service->AccessPoint(ssid, password, mac);
        } else {
            wifi_service->CancelAccessPoint(ssid);
        }
        return;
    }

    if (BindedToDateTime()) {
        return;
    }
}

void ControllerVariable::BindToSecureWiFi(WiFiService *wifi_service,
                                          const char *ssid,
                                          const char *password,
                                          const char *mac) {
    Unbind();
    this->wifi_service = wifi_service;
    this->ssid = ssid;
    this->password = password;
    this->mac = mac;
}

void ControllerVariable::BindToInsecureWiFi(WiFiService *wifi_service, const char *ssid) {
    Unbind();
    this->wifi_service = wifi_service;
    this->ssid = ssid;
    this->password = NULL;
    this->mac = NULL;
}

void ControllerVariable::BindToStaWiFi(WiFiService *wifi_service) {
    Unbind();
    this->wifi_service = wifi_service;
    this->ssid = NULL;
    this->password = NULL;
    this->mac = NULL;
}

void ControllerVariable::Unbind() {
    CancelReadingProcess();
    this->wifi_service = NULL;
    this->datetime_service = NULL;
}

bool ControllerVariable::BindedToWiFi() {
    return this->wifi_service != NULL;
}

void ControllerVariable::CancelReadingProcess() {
    log_d(TAG_ControllerVariable,
          "CancelReadingProcess, wifi:%u, date_time:%u, required:%u",
          BindedToWiFi(),
          BindedToDateTime(),
          required_reading);
    if (BindedToWiFi()) {
        bool insecure_scan = ssid != NULL && password == NULL && mac == NULL;
        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;

        if (insecure_scan) {
            wifi_service->CancelScan(ssid);
        } else if (secure_ap) {
            wifi_service->CancelAccessPoint(ssid);
        } else {
            wifi_service->DisconnectFromStation();
        }
        value = LogicElement::MinValue;
        out_value = LogicElement::MinValue;
        return;
    }

    if (BindedToDateTime()) {
        value = LogicElement::MinValue;
        out_value = LogicElement::MinValue;
        return;
    }
}

void ControllerVariable::BindToDateTime(DatetimeService *datetime_service,
                                        DatetimePart datetime_part) {
    Unbind();
    this->datetime_service = datetime_service;
    this->datetime_part = datetime_part;
}

bool ControllerVariable::BindedToDateTime() {
    return this->datetime_service != NULL;
}