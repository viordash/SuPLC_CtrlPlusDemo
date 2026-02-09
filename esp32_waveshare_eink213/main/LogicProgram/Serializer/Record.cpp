#include "LogicProgram/Serializer/Record.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool Record::Write(void *data,
                   size_t data_size,
                   uint8_t *buffer,
                   size_t buffer_size,
                   size_t *writed) {

    bool just_obtain_size = buffer == NULL;
    if (!just_obtain_size) {
        if (buffer_size - *writed < data_size) {
            return false;
        }
        memcpy(&buffer[*writed], data, data_size);
    }
    *writed += data_size;
    return true;
}

bool Record::Read(void *data,
                  size_t data_size,
                  uint8_t *buffer,
                  size_t buffer_size,
                  size_t *readed) {
    if (buffer_size - *readed < data_size) {
        return false;
    }
    memcpy(data, &buffer[*readed], data_size);
    *readed += data_size;
    return true;
}