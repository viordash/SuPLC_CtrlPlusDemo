
#include "board.h"
#include "esp_spiffs.h"
#include "os.h"

static const char *TAG_storage = "storage";

void open_storage(const char *partition, const char *path) {
    log_d(TAG_storage, "Initializing storage '%s'", partition);

    esp_vfs_spiffs_conf_t conf = { .base_path = path,
                                   .partition_label = partition,
                                   .max_files = 3,
                                   .format_if_mount_failed = true };

    ERROR_CHECK(esp_vfs_spiffs_register(&conf));
}

void close_storage(const char *partition) {
    log_d(TAG_storage, "unmount storage '%s'", partition);
    ERROR_CHECK(esp_vfs_spiffs_unregister(partition));
}