#pragma once

#include <stdint.h>
#include <unistd.h>

class ConverterUint32 {
  public:
    static bool FromString(const char *val, uint32_t *out);
    static bool ToString(const uint32_t val, char *out, int outSize);
};
