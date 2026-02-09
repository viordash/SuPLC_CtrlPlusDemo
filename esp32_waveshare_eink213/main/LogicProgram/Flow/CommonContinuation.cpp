#include "LogicProgram/Flow/CommonContinuation.h"
#include "LogicProgram/Serializer/Record.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonContinuation::CommonContinuation() : LogicElement() {
}

CommonContinuation::~CommonContinuation() {
}

size_t CommonContinuation::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    uint8_t dummy = 0;
    if (!Record::Write(&dummy, sizeof(dummy), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonContinuation::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t dummy;
    if (!Record::Read(&dummy, sizeof(dummy), buffer, buffer_size, &readed)) {
        return 0;
    }
    return readed;
}

void CommonContinuation::SelectPrior() {
}

void CommonContinuation::SelectNext() {
}

void CommonContinuation::PageUp() {
}

void CommonContinuation::PageDown() {
}

void CommonContinuation::Change() {
    EndEditing();
}

void CommonContinuation::Option() {
}
