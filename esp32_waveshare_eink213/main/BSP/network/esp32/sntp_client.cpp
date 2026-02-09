

#include "board.h"
#include "esp_sntp.h"
#include "os.h"

static const char *TAG_sntp = "sntp";

static struct {
    char primary_server[64];
    char secondary_server[64];
    sntp_time_sync_event_handler_t time_sync_event_handler;
    void *event_handler_arg;
} sntp;

static void time_sync_notification_cb(struct timeval *tv);

void start_sntp(sntp_config_t *config) {
    log_d(TAG_sntp,
          "start, serv_0:%s, serv_1:%s",
          config->primary_server,
          config->secondary_server);

    strncpy(sntp.primary_server, config->primary_server, sizeof(sntp.primary_server) - 1);
    sntp.primary_server[sizeof(sntp.primary_server) - 1] = 0;
    strncpy(sntp.secondary_server, config->secondary_server, sizeof(sntp.secondary_server) - 1);
    sntp.secondary_server[sizeof(sntp.secondary_server) - 1] = 0;
    sntp.time_sync_event_handler = config->time_sync_event_handler;
    sntp.event_handler_arg = config->event_handler_arg;

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    if (strlen(sntp.primary_server) > 0) {
        esp_sntp_setservername(0, sntp.primary_server);
    }
    if (strlen(sntp.secondary_server) > 0) {
        esp_sntp_setservername(1, sntp.secondary_server);
    }
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}

void stop_sntp() {
    log_d(TAG_sntp, "stop");
    esp_sntp_stop();
    sntp_set_time_sync_notification_cb(NULL);
}

bool sntp_in_process() {
    log_d(TAG_sntp,
          "sntp_in_process, serv_0:%u, serv_1:%u",
          (unsigned int)esp_sntp_getreachability(0),
          (unsigned int)esp_sntp_getreachability(1));
    return esp_sntp_getreachability(0) != 0 || esp_sntp_getreachability(1) != 0;
}

static void time_sync_notification_cb(struct timeval *tv) {
    log_d(TAG_sntp, "time_sync_notification_cb: %u", (unsigned int)tv->tv_sec);
    sntp.time_sync_event_handler(sntp.event_handler_arg, tv);
}
