#pragma once

#include <stdint.h>
#include <unistd.h>

class StringHelper {
  public:
    static char *Duplicate(const char *src);
    static bool HasHexPrefix(const char *val);
};
