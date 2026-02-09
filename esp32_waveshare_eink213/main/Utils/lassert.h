#pragma once

#include <cassert>

#define ASSERT(x)                                                                                  \
    do {                                                                                           \
        bool cond = (x);                                                                           \
        (void)cond;                                                                                \
        assert(cond);                                                                              \
    } while (0);
