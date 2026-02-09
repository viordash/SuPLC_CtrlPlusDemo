#include "LogicProgram/LogicElement.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicElement::LogicElement() : EditableElement() {
    this->state = LogicItemState::lisPassive;
}

LogicElement::~LogicElement() {
}

LogicItemState LogicElement::GetState() {
    return state;
}