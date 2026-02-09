#pragma once

#include <stdint.h>
#include <unistd.h>

typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t version;
} backups_storage;

bool backups_storage_load(const char *name, backups_storage *storage);
void backups_storage_store(const char *name, backups_storage *storage);
void backups_storage_delete(const char *name);