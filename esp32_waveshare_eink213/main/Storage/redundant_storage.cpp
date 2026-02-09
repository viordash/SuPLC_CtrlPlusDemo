
#include "redundant_storage.h"
#include "Hashing/crc32.h"
#include "board.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    uint32_t crc;
} redundant_storage_header;

static const char *TAG_R = "redundant_storage";

static bool read_file(FILE *file, redundant_storage *storage) {
    if (file == NULL) {
        log_e(TAG_R, "check_file, file not exists");
        return false;
    }

    struct stat st;

    if (fstat(fileno(file), &st) != 0 || (size_t)st.st_size < sizeof(redundant_storage_header)) {
        log_e(TAG_R, "check_file, file size wrong");
        return false;
    }

    redundant_storage_header header;
    if (fread(&header, 1, sizeof(redundant_storage_header), file)
        != sizeof(redundant_storage_header)) {
        log_e(TAG_R, "check_file, read header error");
        return false;
    }

    if (fread(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)) {
        log_e(TAG_R, "check_file, read version error");
        return storage;
    }
    uint32_t crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));

    storage->size = st.st_size - sizeof(redundant_storage_header) - sizeof(storage->version);
    storage->data = new uint8_t[storage->size];

    if (fread(storage->data, 1, storage->size, file) != storage->size) {
        log_e(TAG_R, "check_file, read data error");
        delete[] storage->data;
        return false;
    }

    if (header.crc != calc_crc32(crc, storage->data, storage->size)) {
        log_w(TAG_R, "check_file, wrong crc\r");
        delete[] storage->data;
        return false;
    }

    return true;
}

static void write_file(const char *path, redundant_storage *storage) {
    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        log_e(TAG_R, "write_file, file open error");
        return;
    }

    redundant_storage_header header;

    header.crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));
    header.crc = calc_crc32(header.crc, storage->data, storage->size);

    if (fwrite(&header, 1, sizeof(redundant_storage_header), file)
            != sizeof(redundant_storage_header)
        || fwrite(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)
        || fwrite(storage->data, 1, storage->size, file) != storage->size) {
        log_e(TAG_R, "write_file, write error");
    }

    fclose(file);
}

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1,
                                         const char *name) {
    log_i(TAG_R,
          "redundant_storage load path_0:'%s', path_1:'%s', name:'%s'",
          path_0,
          path_1,
          name);

    char filename_0[256];
    char filename_1[256];

    open_storage(partition_0, path_0);
    snprintf(filename_0, sizeof(filename_0), "%s/%s", path_0, name);
    FILE *file_0 = fopen(filename_0, "rb");
    redundant_storage storage_0;
    bool storage_0_ready = read_file(file_0, &storage_0);
    if (!storage_0_ready) {
        memset(&storage_0, 0, sizeof(storage_0));
    }

    if (file_0 != NULL) {
        fclose(file_0);
    }
    close_storage(partition_0);

    open_storage(partition_1, path_1);
    snprintf(filename_1, sizeof(filename_1), "%s/%s", path_1, name);
    FILE *file_1 = fopen(filename_1, "rb");
    redundant_storage storage_1;
    bool storage_1_ready = read_file(file_1, &storage_1);
    if (!storage_1_ready) {
        memset(&storage_1, 0, sizeof(storage_1));
    }

    if (file_1 != NULL) {
        fclose(file_1);
    }
    close_storage(partition_1);

    if (storage_0_ready) {
        if (!storage_1_ready) {
            write_file(filename_1, &storage_0);
        } else {
            delete[] storage_1.data;
        }
    }

    if (!storage_0_ready && storage_1_ready) {
        write_file(filename_0, &storage_1);
        storage_0 = storage_1;
    }

    return storage_0;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage *storage) {

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path_0, name);

    open_storage(partition_0, path_0);
    write_file(filename, storage);
    close_storage(partition_0);

    snprintf(filename, sizeof(filename), "%s/%s", path_1, name);
    open_storage(partition_1, path_1);
    write_file(filename, storage);
    close_storage(partition_1);
}

void redundant_storage_delete(const char *partition_0,
                              const char *path_0,
                              const char *partition_1,
                              const char *path_1,
                              const char *name) {

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path_0, name);

    open_storage(partition_0, path_0);
    if (remove(filename) != 0) {
        log_e(TAG_R, "Error deleting file '%s'", filename);
    }
    close_storage(partition_0);

    snprintf(filename, sizeof(filename), "%s/%s", path_1, name);
    open_storage(partition_1, path_1);
    if (remove(filename) != 0) {
        log_e(TAG_R, "Error deleting file '%s'", filename);
    }
    close_storage(partition_1);
}