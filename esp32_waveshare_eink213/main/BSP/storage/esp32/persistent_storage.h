#pragma once

#include <stdint.h>
#include <unistd.h>

void open_storage(const char *partition, const char *path);
void close_storage(const char *partition);