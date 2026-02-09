#pragma once

#include "Display/Common.h"
#include "Display/Font.h"
#include <stdint.h>
#include <unistd.h>

class LabeledElement {
  protected:
    const char *label;
    uint16_t label_width;
    FontLarge label_font;

    void SetLabel(const char *text);

  public:
    explicit LabeledElement();
    const char *GetLabel();
};
