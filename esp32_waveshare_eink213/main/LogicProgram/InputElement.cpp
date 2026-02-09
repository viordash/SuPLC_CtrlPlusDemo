#include "LogicProgram/InputElement.h"
#include "DeviceIO.h"
#include "LogicProgram/LogicProgram.h"
#include "os.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputElement = "InputElement";

InputElement::InputElement() {
    Input = NULL;
}

InputElement::~InputElement() {
}

void InputElement::SetIoAdr(const MapIO io_adr) {
    auto it_di = DeviceIO::DigitalInputs.find(io_adr);
    if (it_di != DeviceIO::DigitalInputs.end()) {
        Input = (*it_di).second;
        return;
    }

    auto it_do = DeviceIO::DigitalOutputs.find(io_adr);
    if (it_do != DeviceIO::DigitalOutputs.end()) {
        Input = (*it_do).second;
        return;
    }

    auto it_ai = DeviceIO::AnalogInputs.find(io_adr);
    if (it_ai != DeviceIO::AnalogInputs.end()) {
        Input = (*it_ai).second;
        return;
    }

    auto it_var = DeviceIO::Variables.find(io_adr);
    if (it_var != DeviceIO::Variables.end()) {
        Input = (*it_var).second;
        return;
    }

    log_e(TAG_InputElement, "SetIoAdr not found: %d", (int)io_adr);
    Input = NULL;
}

template <class T> bool InputElement::FindIoAdr(T container, MapIO *io_adr) {
    const auto it =
        std::find_if(container.begin(),
                     container.end(),
                     [this](const std::pair<const MapIO, ControllerBaseInput *> &input) {
                         return this->Input == input.second;
                     });
    if (it == container.end()) {
        return false;
    }
    *io_adr = (*it).first;
    return true;
}

MapIO InputElement::GetIoAdr() {
    MapIO io_adr = (MapIO)-1;
    if (FindIoAdr(DeviceIO::DigitalInputs, &io_adr)) {
        return io_adr;
    }
    if (FindIoAdr(DeviceIO::DigitalOutputs, &io_adr)) {
        return io_adr;
    }
    if (FindIoAdr(DeviceIO::AnalogInputs, &io_adr)) {
        return io_adr;
    }
    if (FindIoAdr(DeviceIO::Variables, &io_adr)) {
        return io_adr;
    }
    log_e(TAG_InputElement, "GetIoAdr not found");
    return io_adr;
}

InputElement *InputElement::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_InputNC:
        case TvElementType::et_InputNO:
            return static_cast<CommonInput *>(logic_element);

        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorNe:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonComparator *>(logic_element);

        case TvElementType::et_Indicator:
            return static_cast<Indicator *>(logic_element);

        case TvElementType::et_WiFiBinding:
            return static_cast<WiFiBinding *>(logic_element);

        case TvElementType::et_WiFiStaBinding:
            return static_cast<WiFiStaBinding *>(logic_element);

        case TvElementType::et_WiFiApBinding:
            return static_cast<WiFiApBinding *>(logic_element);

        case TvElementType::et_DateTimeBinding:
            return static_cast<DateTimeBinding *>(logic_element);

        case TvElementType::et_TrackBarControl:
            return static_cast<TrackBarControl *>(logic_element);

        default:
            return NULL;
    }
}