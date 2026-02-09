#pragma once

#include "Display/ListBox.h"
#include <stdint.h>
#include <unistd.h>

class LogsList : public ListBox {
  protected:
    int curr_line;

  public:
    explicit LogsList(const char *title);

    void Append(const char *message);
};
