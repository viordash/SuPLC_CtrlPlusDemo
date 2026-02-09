#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/LogicElement.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerDO = "ControllerDO";

ControllerDO::ControllerDO(gpio_output gpio, const char *name) : ControllerBaseInputOutput(name) {
    this->gpio = gpio;
}

void ControllerDO::FetchValue() {
    bool val_1bit = get_digital_output_value(gpio);
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;

    log_d(TAG_ControllerDO, "FetchValue, percent04:%u, in_value:%u", percent04, PeekValue());
    UpdateValue(percent04);
}

void ControllerDO::CommitChanges() {
    if (!required_writing) {
        return;
    }
    required_writing = false;
    set_digital_output_value(gpio, out_value != LogicElement::MinValue);
    UpdateValue(out_value);
}
