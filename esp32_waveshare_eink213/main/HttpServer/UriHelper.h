#pragma once

#include <stdint.h>
#include <unistd.h>

class UriHelper {
  public:
    static const char *GetPathFromUri(char *dest, const char *uri, size_t destSize);
    static const char *GetContentType(const char *filename);
    static bool GetPathParameterU32(const char *pattern, const char *uri, uint32_t *outVal);
};