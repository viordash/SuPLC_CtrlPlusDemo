#pragma once

#include <stdint.h>
#include <unistd.h>

class Record {

  public:
    static bool
    Write(void *data, size_t data_size, uint8_t *buffer, size_t buffer_size, size_t *writed);
    static bool
    Read(void *data, size_t data_size, uint8_t *buffer, size_t buffer_size, size_t *readed);
};
