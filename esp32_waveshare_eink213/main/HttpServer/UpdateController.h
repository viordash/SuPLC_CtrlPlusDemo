#pragma once

#include "BaseController.h"
#include "board.h"
#include "os.h"

class UpdateController : public BaseController {
  public:
    UpdateController();
    ~UpdateController();

    std::vector<httpd_uri_t> GetUriHandlers() override;

    static error_t Handler(httpd_req_t *req);

  private:
    bool ReceiveFile(httpd_req_t *req, char *buffer);
    bool
    BeginOta(const partition_t **update_partition, ota_handle_t *update_handle, size_t image_size);
    bool FinishOta(const partition_t *update_partition, ota_handle_t *update_handle);
};