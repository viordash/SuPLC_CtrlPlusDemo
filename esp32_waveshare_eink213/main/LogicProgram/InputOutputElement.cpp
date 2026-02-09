#include "InputOutputElement.h"
#include "DeviceIO.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputOutputElement = "InputOutputElement";

InputOutputElement::InputOutputElement() : InputElement() {
    Output = NULL;
}

InputOutputElement::~InputOutputElement() {
}

void InputOutputElement::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);

    auto it_do = DeviceIO::DigitalOutputs.find(io_adr);
    if (it_do != DeviceIO::DigitalOutputs.end()) {
        Output = (*it_do).second;
        return;
    }

    auto it_var = DeviceIO::Variables.find(io_adr);
    if (it_var != DeviceIO::Variables.end()) {
        Output = (*it_var).second;
        return;
    }

    log_e(TAG_InputOutputElement, "SetIoAdr not found: %d", (int)io_adr);
    Output = NULL;
}
