#include "LogicProgram/ControllerBaseInputOutput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInputOutput::ControllerBaseInputOutput(const char *name) : ControllerBaseInput(name) {
    required_writing = false;
    out_value = LogicElement::MinValue;
}

ControllerBaseInputOutput::~ControllerBaseInputOutput() {
}

void ControllerBaseInputOutput::Init() {
    ControllerBaseInput::Init();
    required_writing = false;
    out_value = LogicElement::MinValue;
}

void ControllerBaseInputOutput::WriteValue(uint8_t new_value) {
    required_writing = true;
    out_value = new_value;
}