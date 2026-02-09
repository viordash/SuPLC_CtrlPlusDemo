
#include "ConverterUint32.h"
#include "StringHelper.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

bool ConverterUint32::FromString(const char *val, uint32_t *out) {
    if (out == NULL || val == NULL) {
        return false;
    }
    char *endptr;
    uint32_t converted;

    errno = 0;
    converted = strtoul(val, &endptr, StringHelper::HasHexPrefix(val) ? 16 : 10);

    if ((errno == ERANGE && converted == ULONG_MAX) || (errno != 0 && converted == 0)) {
        return false;
    }

    if (endptr == val) {
        return false;
    }
    *out = converted;
    return true;
}

bool ConverterUint32::ToString(const uint32_t val, char *out, int outSize) {
    if (out == NULL) {
        return false;
    }

    snprintf(out, outSize, "%u", (unsigned)val);
    return true;
}
