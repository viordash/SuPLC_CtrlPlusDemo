#include "BaseController.h"
#include "os.h"
#include <string.h>

static const char *TAG_BaseController = "base_controller";

BaseController::BaseController() {
}

BaseController::~BaseController() {
}

error_t BaseController::GetUrlQueryParamValue(httpd_req_t *req,
                                              const char *key,
                                              char *value,
                                              size_t valueSize) {
    char *buf;
    error_t res;
    size_t buf_len;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len <= 1) {
        return STATUS_FAIL;
    }
    buf = new char[buf_len];
    if ((res = httpd_req_get_url_query_str(req, buf, buf_len)) == STATUS_OK) {
        if ((res = httpd_query_key_value(buf, key, value, valueSize)) == STATUS_OK) {
            log_d(TAG_BaseController, "Found URL query parameter => %s=%s", key, value);
        }
    }
    delete[] buf;

    return res;
}
