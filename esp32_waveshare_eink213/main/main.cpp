#include "Datetime/DatetimeService.h"
#include "Display/RenderingService.h"
#include "Display/display.h"
#include "Hashing/crc32.h"
#include "Hotreload/hotreload_service.h"
#include "Hotreload/restart_counter.h"
#include "HttpServer/http_server.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "Maintenance/ServiceModeHandler.h"
#include "Settings/settings.h"
#include "WiFi/WiFiService.h"
#include "board.h"
#include "buttons.h"
#include "os.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "main";

extern "C" void app_main();

extern CurrentSettings::device_settings settings;

void app_main() {
    printf("---------- Start app ----------\n");
    load_hotreload();

    t_board board = board_init();
    display_init();
    print_board_info();

    load_settings();

    hot_restart_counter();

    if (!hotreload->is_hotstart && maintenance_button_pressed()) {
        ServiceModeHandler::Start(&board);
    }

    DatetimeService datetime_service;
    RenderingService rendering_service(board.display_events);

    WiFiService wifi_service([&rendering_service]() { start_http_server(rendering_service); },
                             []() { stop_http_server(); });

    wifi_service.Start();
    datetime_service.Start();
    if (hotreload->is_hotstart) {
        log_i(TAG, "is hotstart");
        setenv("TZ", settings.datetime.timezone, 1);
        tzset();
        datetime_service.ManualSet(&hotreload->current_datetime);
    }
    ProcessWakeupService process_wakeup_service;
    Controller::Start(board.gpio_events,
                      &wifi_service,
                      &rendering_service,
                      &datetime_service,
                      &process_wakeup_service);

    uint32_t free_mem = get_free_heap_size();
    printf("mem: %u\n", (unsigned int)free_mem);
    while (true) {
        sleep_ms(1000);
        uint32_t curr_free_mem = get_free_heap_size();
        int32_t dead_band_1perc = curr_free_mem / 100;
        int32_t diff = curr_free_mem - free_mem;
        if (diff > dead_band_1perc || diff < -dead_band_1perc) {
            printf("mem: %u\n", (unsigned int)curr_free_mem);
            free_mem = curr_free_mem;
        }
    }
}
