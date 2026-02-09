
#include "Hotreload/hotreload_service.h"
#include "Hashing/crc32.h"
#include "board.h"
#include "os.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG_hotreload = "hotreload";

static const uint32_t MAGIC = 0xDE4572BB;

typedef uint32_t t_crc;

typedef struct {
    hotreload_data data;
    union {
        uint32_t magic;
        uint8_t dummy[256 - sizeof(hotreload_data) - sizeof(t_crc)];
    };
    t_crc crc;
} rtc_hotreload_data;

RTC_NOINIT_ATTR rtc_hotreload_data _rtc_hotreload_data;
hotreload_data *hotreload = NULL;
static mutex_handle_t mutex = NULL;

static_assert(sizeof(hotreload->is_hotstart) == 4, "sizeof(is_hotstart)");
static_assert(sizeof(hotreload->restart_count) == 4, "sizeof(restart_count)");
static_assert(sizeof(hotreload->view_top_index) == 4, "sizeof(view_top_index)");
static_assert(sizeof(hotreload->selected_network) == 4, "sizeof(selected_network)");
static_assert(sizeof(hotreload->current_datetime.year) == 4, "sizeof(current_datetime.year)");
static_assert(sizeof(hotreload->current_datetime.month) == 4, "sizeof(current_datetime.month)");
static_assert(sizeof(hotreload->current_datetime.day) == 4, "sizeof(current_datetime.day)");
static_assert(sizeof(hotreload->current_datetime.hour) == 4, "sizeof(current_datetime.hour)");
static_assert(sizeof(hotreload->current_datetime.minute) == 4, "sizeof(current_datetime.minute)");
static_assert(sizeof(hotreload->current_datetime.second) == 4, "sizeof(current_datetime.second)");

void init_hotreload() {
    hotreload->is_hotstart = false;
    hotreload->restart_count = 0;
    hotreload->view_top_index = 0;
    hotreload->selected_network = -1;

    hotreload->current_datetime.year = 2025;
    hotreload->current_datetime.month = 01;
    hotreload->current_datetime.day = 01;
    hotreload->current_datetime.hour = 00;
    hotreload->current_datetime.minute = 00;
    hotreload->current_datetime.second = 00;
}

void load_hotreload() {
    ERROR_CHECK(mutex == NULL ? STATUS_OK : ERR_NO_MEM);
    mutex = create_mutex();

    hotreload = (hotreload_data *)&_rtc_hotreload_data.data;
    if (_rtc_hotreload_data.magic != MAGIC) {
        log_w(TAG_hotreload, "try_load_hotreload, incorrect magic");
        init_hotreload();
        return;
    }

    t_crc crc = calc_crc32(CRC32_INIT,
                           (const void *)&_rtc_hotreload_data.data,
                           sizeof(_rtc_hotreload_data.data));
    crc = calc_crc32(crc,
                     (const void *)&_rtc_hotreload_data.dummy,
                     sizeof(_rtc_hotreload_data.dummy));

    if (_rtc_hotreload_data.crc != crc) {
        log_w(TAG_hotreload, "try_load_hotreload, wrong crc");
        init_hotreload();
        return;
    }
    if (!ValidateDatetime(&hotreload->current_datetime)) {
        log_w(TAG_hotreload, "try_load_hotreload, wrong datetime");
        init_hotreload();
        return;
    }
    hotreload->is_hotstart = true;
}

void store_hotreload() {
    _rtc_hotreload_data.magic = MAGIC;
    t_crc crc = calc_crc32(CRC32_INIT, (const void *)hotreload, sizeof(*hotreload));
    crc = calc_crc32(crc,
                     (const void *)&_rtc_hotreload_data.dummy,
                     sizeof(_rtc_hotreload_data.dummy));
    _rtc_hotreload_data.crc = crc;
}

void lock_hotreload() {
    ERROR_CHECK(!lock_mutex(mutex) ? ERR_NO_MEM : STATUS_OK);
}

void unlock_hotreload() {
    unlock_mutex(mutex);
}