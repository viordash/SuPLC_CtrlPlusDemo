#pragma once

#include "BaseController.h"
#include "board.h"
#include "os.h"

class InputController : public BaseController {
    struct DataPaging {
        uint16_t offset;
        uint16_t count;
    };

  public:
    InputController();
    ~InputController();

    std::vector<httpd_uri_t> GetUriHandlers() override;

    static error_t KeyPress(httpd_req_t *req);

  private:
};