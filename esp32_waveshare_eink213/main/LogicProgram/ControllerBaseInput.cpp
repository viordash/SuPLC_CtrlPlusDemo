#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInput::ControllerBaseInput(const char *name) {
    this->name = name;
    required_reading = true;
    value = LogicElement::MinValue;
}

ControllerBaseInput::~ControllerBaseInput() {
}

void ControllerBaseInput::Init() {
    required_reading = true;
    value = LogicElement::MinValue;
}

uint8_t ControllerBaseInput::ReadValue() {
    required_reading = true;
    return value;
}

uint8_t ControllerBaseInput::PeekValue() {
    std::lock_guard<std::mutex> lock(lock_value);
    return value;
}

void ControllerBaseInput::UpdateValue(uint8_t new_value) {
    std::lock_guard<std::mutex> lock(lock_value);
    value = new_value;
}

const char *ControllerBaseInput::GetName() {
    return name;
}