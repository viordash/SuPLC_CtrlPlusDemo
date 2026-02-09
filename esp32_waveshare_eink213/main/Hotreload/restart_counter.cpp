
#include "restart_counter.h"
#include "Hotreload/hotreload_service.h"
#include "os.h"
#include <string.h>

static const char *TAG_restart_counter = "restart_counter";

void hot_restart_counter() {
    SAFETY_HOTRELOAD({
        hotreload->restart_count++;
        store_hotreload();
    });

    log_i(TAG_restart_counter, "count:%u", (unsigned int)hotreload->restart_count);
}