#include "DisplayController.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "os.h"
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG_DisplayController = "DisplayController";

DisplayController::DisplayController(RenderingService &rendering_service)
    : BaseController(), rendering_service{ rendering_service } {
}

DisplayController::~DisplayController() {
}

std::vector<httpd_uri_t> DisplayController::GetUriHandlers() {
    std::vector<httpd_uri_t> handlers = { {
                                              .uri = "/devconfig",
                                              .method = HTTP_GET,
                                              .handler = DisplayController::GetDeviceConfig,
                                              .user_ctx = this //
                                          },
                                          {
                                              .uri = "/bitmap",
                                              .method = HTTP_GET,
                                              .handler = DisplayController::GetBitmap,
                                              .user_ctx = this //
                                          } };
    return handlers;
}

error_t DisplayController::GetDeviceConfig(httpd_req_t *req) {
    char response[256];
    int size = snprintf(response,
                        sizeof(response),
                        "{\"display\":{\"height\":%u,\"width\":%u,\"id\":\"%s\",\"area_top\":%u,"
                        "\"area_width\":%u,\"rails_height\":%u},\"request_limit\":%u}",
                        DISPLAY_HEIGHT,
                        DISPLAY_WIDTH,
                        DISPLAY_ID,
                        RailsTop,
                        SCROLLBAR_LEFT,
                        RailsHeight,
                        Http_RequestsLimit);

    log_d(TAG_DisplayController, "GetDeviceConfig : %s", response);
    error_t res = httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    if (res != STATUS_OK) {
        return res;
    }
    return httpd_resp_send(req, response, size);
}

error_t DisplayController::GetBitmap(httpd_req_t *req) {
    auto controller = static_cast<DisplayController *>(req->user_ctx);
    log_d(TAG_DisplayController, "GetBitmap request received");

    error_t res;
    char etag_s[64];
    char if_none_match[64];

    res = httpd_req_get_hdr_value_str(req, "If-None-Match", if_none_match, sizeof(if_none_match));
    if (res != STATUS_OK) {
        if_none_match[0] = 0;
    }

    auto cached_bitmap = controller->rendering_service.BeginRenderOnExternal();
    sprintf(etag_s, "%u", (unsigned)cached_bitmap.last_change_time_ms);

    if (strcmp(if_none_match, etag_s) != 0) {
        res = controller->CreateBitmapDataResponse(req,
                                                   cached_bitmap.bitmap,
                                                   etag_s,
                                                   cached_bitmap.view_offset,
                                                   cached_bitmap.view_count);
        // log_i(TAG_DisplayController, "-----------------");
        // for (size_t i = 0; i < 16 * 16; i += 16) {
        //     log_i(TAG_DisplayController,
        //           "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        //           cached_bitmap.bitmap[i + 0],
        //           cached_bitmap.bitmap[i + 1],
        //           cached_bitmap.bitmap[i + 2],
        //           cached_bitmap.bitmap[i + 3],
        //           cached_bitmap.bitmap[i + 4],
        //           cached_bitmap.bitmap[i + 5],
        //           cached_bitmap.bitmap[i + 6],
        //           cached_bitmap.bitmap[i + 7],
        //           cached_bitmap.bitmap[i + 8],
        //           cached_bitmap.bitmap[i + 9],
        //           cached_bitmap.bitmap[i + 10],
        //           cached_bitmap.bitmap[i + 11],
        //           cached_bitmap.bitmap[i + 12],
        //           cached_bitmap.bitmap[i + 13],
        //           cached_bitmap.bitmap[i + 14],
        //           cached_bitmap.bitmap[i + 15]);
        // }
        // log_i(TAG_DisplayController, "-----------------");
    } else {
        res = controller->CreateNotModifiedResponse(req, etag_s);
    }
    controller->rendering_service.EndRenderOnExternal();
    return res;
}

error_t DisplayController::CreateBitmapDataResponse(httpd_req_t *req,
                                                    uint8_t *bitmap,
                                                    const char *etag,
                                                    int32_t view_offset,
                                                    uint32_t view_count) {
    log_d(TAG_DisplayController, "CreateBitmapDataResponse");
    error_t res = httpd_resp_set_type(req, HTTPD_TYPE_OCTET);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_type error");
        return res;
    }

    res = httpd_resp_set_hdr(req, "ETag", etag);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_hdr 'ETag' error");
        return res;
    }

    char data_paging[256];
    snprintf(data_paging,
             sizeof(data_paging),
             "{\"offset\":%d,\"count\":%u}",
             (signed)view_offset,
             (unsigned)view_count);
    res = httpd_resp_set_hdr(req, "X-DataPaging", data_paging);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController,
              "CreateBitmapDataResponse httpd_resp_set_hdr 'X-DataPaging' error");
        return res;
    }

    res = httpd_resp_send(req, (const char *)bitmap, DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_hdr error");
        return res;
    }
    return res;
}

error_t DisplayController::CreateNotModifiedResponse(httpd_req_t *req, const char *etag) {
    log_d(TAG_DisplayController, "CreateNotModifiedResponse");
    error_t res = httpd_resp_set_status(req, "304 Not Modified");
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_set_type error");
        return res;
    }

    res = httpd_resp_set_hdr(req, "ETag", etag);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateNotModifiedResponse httpd_resp_set_hdr 'ETag' error");
        return res;
    }

    res = httpd_resp_send(req, NULL, 0);
    if (res != STATUS_OK) {
        log_e(TAG_DisplayController, "CreateBitmapDataResponse httpd_resp_send error");
        return res;
    }
    return res;
}
