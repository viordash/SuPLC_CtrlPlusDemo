

#include "StringHelper.h"
#include <string.h>

char *StringHelper::Duplicate(const char *src) {
    if (src == NULL) {
        return NULL;
    }
    int len = strlen(src) + 1;
    char *dest = new char[len];
    memcpy(dest, src, len);
    return dest;
}

bool StringHelper::HasHexPrefix(const char *val) {
    if (val == NULL) {
        return false;
    }
    char *str = (char *)val;
    char ch;
    while (((ch = *str) == ' ' || ch == '\t')) {
        str++;
    }
    return str[0] == '0' && (str[1] == 'x' || str[1] == 'X');
}