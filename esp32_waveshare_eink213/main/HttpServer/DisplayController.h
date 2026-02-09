#pragma once

#include "BaseController.h"
#include "Display/RenderingService.h"
#include "board.h"
#include "os.h"

class DisplayController : public BaseController {
    struct DataPaging {
        uint16_t offset;
        uint16_t count;
    };

  public:
    DisplayController(RenderingService &rendering_service);
    ~DisplayController();

    std::vector<httpd_uri_t> GetUriHandlers() override;

    static error_t GetDeviceConfig(httpd_req_t *req);
    static error_t GetBitmap(httpd_req_t *req);
    error_t CreateBitmapDataResponse(httpd_req_t *req,
                                     uint8_t *bitmap,
                                     const char *etag,
                                     int32_t view_offset,
                                     uint32_t view_count);
    error_t CreateNotModifiedResponse(httpd_req_t *req, const char *etag);

  private:
    RenderingService &rendering_service;
};