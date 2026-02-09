#include "Display/LabeledElement.h"
#include "Display/display.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LabeledElement::LabeledElement() {
    label = NULL;
    label_width = 0;
}

void LabeledElement::SetLabel(const char *text) {
    label = text;
    label_width = label_font.MeasureWidth(label);
}

const char *LabeledElement::GetLabel() {
    return this->label;
}