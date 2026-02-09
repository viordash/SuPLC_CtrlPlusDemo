#include "LogicProgram/Serializer/LogicElementSerializer.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_LogicElementSerializer = "LogicElementSerializer";

size_t
LogicElementSerializer::Serialize(LogicElement *element, uint8_t *buffer, size_t buffer_size) {
    log_i(TAG_LogicElementSerializer,
          "Serialize: buffer:%p, size:%u",
          buffer,
          (unsigned int)buffer_size);
    size_t writed = element->Serialize(buffer, buffer_size);

    return writed;
}

size_t
LogicElementSerializer::Deserialize(LogicElement *element, uint8_t *buffer, size_t buffer_size) {
    log_i(TAG_LogicElementSerializer,
          "Deserialize: buffer:%p, size:%u",
          buffer,
          (unsigned int)buffer_size);
    size_t readed = element->Deserialize(buffer, buffer_size);

    return readed;
}
