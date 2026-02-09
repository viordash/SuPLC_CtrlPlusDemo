#pragma once

#include <stdint.h>
#include <unistd.h>

struct Point {
    uint16_t x;
    uint16_t y;
};

struct Size {
    uint16_t width;
    uint16_t height;
};

struct Bitmap {
    const struct Size size;
    const uint8_t data[];
};

struct Rect {
    Point top_left;
    Point bottom_right;
};
