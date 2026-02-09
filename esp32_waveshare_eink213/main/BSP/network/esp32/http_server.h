#pragma once

#include "BSP/os/esp32/tasks.h"
#include <esp_http_server.h>

#define HTTPD_CONFIG_DEFAULT()                                                                     \
    { .task_priority = task_priority_t::tp_Middle,                                                 \
      .stack_size = 4096,                                                                          \
      .core_id = tskNO_AFFINITY,                                                                   \
      .task_caps = (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),                                        \
      .server_port = 80,                                                                           \
      .ctrl_port = 32768,                                                                          \
      .max_open_sockets = 7,                                                                       \
      .max_uri_handlers = 12,                                                                       \
      .max_resp_headers = 8,                                                                       \
      .backlog_conn = 5,                                                                           \
      .lru_purge_enable = false,                                                                   \
      .recv_wait_timeout = 5,                                                                      \
      .send_wait_timeout = 5,                                                                      \
      .global_user_ctx = NULL,                                                                     \
      .global_user_ctx_free_fn = NULL,                                                             \
      .global_transport_ctx = NULL,                                                                \
      .global_transport_ctx_free_fn = NULL,                                                        \
      .enable_so_linger = false,                                                                   \
      .linger_timeout = 0,                                                                         \
      .keep_alive_enable = false,                                                                  \
      .keep_alive_idle = 0,                                                                        \
      .keep_alive_interval = 0,                                                                    \
      .keep_alive_count = 0,                                                                       \
      .open_fn = NULL,                                                                             \
      .close_fn = NULL,                                                                            \
      .uri_match_fn = NULL };

static inline error_t httpd_resp_send_error_400(httpd_req_t *req) {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
}

static inline error_t httpd_resp_send_error_404(httpd_req_t *req) {
    return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, NULL);
}

static inline error_t httpd_resp_send_error_408(httpd_req_t *req) {
    return httpd_resp_send_err(req, HTTPD_408_REQ_TIMEOUT, NULL);
}

static inline error_t httpd_resp_send_error_500(httpd_req_t *req) {
    return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, NULL);
}
