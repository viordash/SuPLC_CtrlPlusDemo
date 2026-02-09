#pragma once

#include "BaseController.h"

class MainController : public BaseController {
  public:
    MainController();
    ~MainController();

    std::vector<httpd_uri_t> GetUriHandlers() override;

  protected:
    static error_t SendingFile(httpd_req_t *req);
};