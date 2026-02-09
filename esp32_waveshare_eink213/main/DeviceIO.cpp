

#include "DeviceIO.h"
#include "Datetime/DatetimeService.h"
#include "LogicProgram/Controller.h"
#include "WiFi/WiFiService.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DeviceIO = "DeviceIO";

template <MapIO map_io> auto make_digital_input_entry(gpio_input gpio, const char *name) {
    return std::make_pair(map_io, new ControllerDI(gpio, name));
}
template <MapIO map_io> auto make_digital_output_entry(gpio_output gpio, const char *name) {
    return std::make_pair(map_io, new ControllerDO(gpio, name));
}
template <MapIO map_io> auto make_analog_input_entry(adc_input adc, const char *name) {
    return std::make_pair(map_io, new ControllerAI(adc, name));
}
template <MapIO map_io> auto make_variable_entry(const char *name) {
    return std::make_pair(map_io, new ControllerVariable(name));
}

std::unordered_map<MapIO, ControllerDI *> DeviceIO::DigitalInputs = {
    make_digital_input_entry<MapIO::D0>(gpio_input::INPUT_0, "D0"),
    make_digital_input_entry<MapIO::D1>(gpio_input::INPUT_1, "D1"),
    make_digital_input_entry<MapIO::D2>(gpio_input::INPUT_2, "D2"),
    make_digital_input_entry<MapIO::D3>(gpio_input::INPUT_3, "D3")
};
std::unordered_map<MapIO, ControllerDO *> DeviceIO::DigitalOutputs = {
    make_digital_output_entry<MapIO::O0>(gpio_output::OUTPUT_0, "O0"),
    make_digital_output_entry<MapIO::O1>(gpio_output::OUTPUT_1, "O1"),
    make_digital_output_entry<MapIO::O2>(gpio_output::OUTPUT_2, "O2"),
    make_digital_output_entry<MapIO::O3>(gpio_output::OUTPUT_3, "O3")
};
std::unordered_map<MapIO, ControllerAI *> DeviceIO::AnalogInputs = {
    make_analog_input_entry<MapIO::A0>(adc_input::CHAN_0, "A0"),
    make_analog_input_entry<MapIO::A1>(adc_input::CHAN_1, "A1"),
    make_analog_input_entry<MapIO::A2>(adc_input::VBAT, "A2")
};
std::unordered_map<MapIO, ControllerVariable *> DeviceIO::Variables = {
    make_variable_entry<MapIO::V1>("V1"),
    make_variable_entry<MapIO::V2>("V2"),
    make_variable_entry<MapIO::V3>("V3"),
    make_variable_entry<MapIO::V4>("V4")
};

void DeviceIO::Init() {
    for (const auto &input : DeviceIO::DigitalInputs) {
        input.second->Init();
    }
    for (const auto &output : DeviceIO::DigitalOutputs) {
        output.second->Init();
    }
    for (const auto &input : DeviceIO::AnalogInputs) {
        input.second->Init();
    }
    for (const auto &var : DeviceIO::Variables) {
        var.second->Init();
    }
}

void DeviceIO::FetchIOValues() {
    for (const auto &input : DeviceIO::DigitalInputs) {
        input.second->FetchValue();
    }
    for (const auto &output : DeviceIO::DigitalOutputs) {
        output.second->FetchValue();
    }
    for (const auto &input : DeviceIO::AnalogInputs) {
        input.second->FetchValue();
    }
    for (const auto &var : DeviceIO::Variables) {
        var.second->FetchValue();
    }
}

void DeviceIO::CommitChanges() {
    for (const auto &output : DeviceIO::DigitalOutputs) {
        output.second->CommitChanges();
    }
    for (const auto &var : DeviceIO::Variables) {
        var.second->CommitChanges();
    }
}

void DeviceIO::BindVariableToSecureWiFi(const MapIO io_adr,
                                        const char *ssid,
                                        const char *password,
                                        const char *mac) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    const auto it = DeviceIO::Variables.find(io_adr);
    if (it == DeviceIO::Variables.end()) {
        log_e(TAG_DeviceIO, "BindVariableToSecureWiFi not found: %d", (int)io_adr);
        return;
    }
    (*it).second->BindToSecureWiFi(Controller::wifi_service, ssid, password, mac);
}

void DeviceIO::BindVariableToInsecureWiFi(const MapIO io_adr, const char *ssid) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    const auto it = DeviceIO::Variables.find(io_adr);
    if (it == DeviceIO::Variables.end()) {
        log_e(TAG_DeviceIO, "BindVariableToInsecureWiFi not found: %d", (int)io_adr);
        return;
    }
    (*it).second->BindToInsecureWiFi(Controller::wifi_service, ssid);
}

void DeviceIO::BindVariableToStaWiFi(const MapIO io_adr) {
    if (Controller::wifi_service == NULL) {
        return;
    }
    const auto it = DeviceIO::Variables.find(io_adr);
    if (it == DeviceIO::Variables.end()) {
        log_e(TAG_DeviceIO, "BindVariableToStaWiFi not found: %d", (int)io_adr);
        return;
    }
    (*it).second->BindToStaWiFi(Controller::wifi_service);
}

void DeviceIO::BindVariableToToDateTime(const MapIO io_adr, DatetimePart datetime_part) {
    if (Controller::datetime_service == NULL) {
        return;
    }
    const auto it = DeviceIO::Variables.find(io_adr);
    if (it == DeviceIO::Variables.end()) {
        log_e(TAG_DeviceIO, "BindVariableToToDateTime not found: %d", (int)io_adr);
        return;
    }
    (*it).second->BindToDateTime(Controller::datetime_service, datetime_part);
}

void DeviceIO::UnbindVariable(const MapIO io_adr) {
    const auto it = DeviceIO::Variables.find(io_adr);
    if (it == DeviceIO::Variables.end()) {
        log_e(TAG_DeviceIO, "UnbindVariable not found: %d", (int)io_adr);
        return;
    }
    (*it).second->Unbind();
}

const char *DeviceIO::GetIOName(MapIO io_adr) {
    const auto it_di = DeviceIO::DigitalInputs.find(io_adr);
    if (it_di != DeviceIO::DigitalInputs.end()) {
        return (*it_di).second->GetName();
    }

    const auto it_do = DeviceIO::DigitalOutputs.find(io_adr);
    if (it_do != DeviceIO::DigitalOutputs.end()) {
        return (*it_do).second->GetName();
    }

    const auto it_ai = DeviceIO::AnalogInputs.find(io_adr);
    if (it_ai != DeviceIO::AnalogInputs.end()) {
        return (*it_ai).second->GetName();
    }

    const auto it_var = DeviceIO::Variables.find(io_adr);
    if (it_var != DeviceIO::Variables.end()) {
        return (*it_var).second->GetName();
    }

    log_e(TAG_DeviceIO, "GetIOName not found: %d", (int)io_adr);
    return NULL;
}