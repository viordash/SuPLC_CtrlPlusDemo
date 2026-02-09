#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerDI::ControllerDI(gpio_input gpio, const char *name) : ControllerBaseInput(name) {
    this->gpio = gpio;
}

void ControllerDI::FetchValue() {
    bool val_1bit = get_digital_input_value(gpio);
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    UpdateValue(percent04);
}
