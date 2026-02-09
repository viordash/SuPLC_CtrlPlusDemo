#pragma once

#include "board.h"
#include "os.h"
#include <vector>

#define HTTPD_RESP_USE_STRLEN -1
class BaseController {
  public:
    BaseController();
    virtual ~BaseController();

    virtual std::vector<httpd_uri_t> GetUriHandlers() = 0;

  protected:
    error_t GetUrlQueryParamValue(httpd_req_t *req, const char *key, char *value, size_t valueSize);

  private:
    error_t SendHttpError(httpd_req_t *req, const char *status, const char *msg);
};