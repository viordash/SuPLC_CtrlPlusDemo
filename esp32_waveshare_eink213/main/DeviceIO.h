#pragma once

#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/ControllerVariable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

enum MapIO : uint8_t {
    min_bound = 0,
    D0,
    D1,
    D2,
    D3,
    A0,
    A1,
    A2,
    O0,
    O1,
    O2,
    O3,
    V1,
    V2,
    V3,
    V4,
    max_bound
};

class WiFiService;
class DatetimeService;
class DeviceIO {
  protected:
  public:
    static std::unordered_map<MapIO, ControllerDI *> DigitalInputs;
    static std::unordered_map<MapIO, ControllerDO *> DigitalOutputs;
    static std::unordered_map<MapIO, ControllerAI *> AnalogInputs;
    static std::unordered_map<MapIO, ControllerVariable *> Variables;

    static void StartBase(WiFiService *wifi_service, DatetimeService *datetime_service);

    static void Init();
    static void FetchIOValues();
    static void CommitChanges();

    static const char *GetIOName(MapIO io_adr);

    static void BindVariableToSecureWiFi(const MapIO io_adr,
                                         const char *ssid,
                                         const char *password,
                                         const char *mac);
    static void BindVariableToInsecureWiFi(const MapIO io_adr, const char *ssid);
    static void BindVariableToStaWiFi(const MapIO io_adr);
    static void BindVariableToToDateTime(const MapIO io_adr, DatetimePart datetime_part);
    static void UnbindVariable(const MapIO io_adr);
};
