#pragma once

#include <stdint.h>
#include <unistd.h>

typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t version;
} redundant_storage;

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1,
                                         const char *name);

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage *storage);

void redundant_storage_delete(const char *partition_0,
                              const char *path_0,
                              const char *partition_1,
                              const char *path_1,
                              const char *name);
