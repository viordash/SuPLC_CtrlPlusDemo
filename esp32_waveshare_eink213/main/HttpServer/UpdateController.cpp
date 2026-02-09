#include "UpdateController.h"
#include "os.h"
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG_UpdateController = "update_controller";

UpdateController::UpdateController() : BaseController() {
}

UpdateController::~UpdateController() {
}

std::vector<httpd_uri_t> UpdateController::GetUriHandlers() {
    std::vector<httpd_uri_t> handlers = { {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = UpdateController::Handler,
        .user_ctx = this //
    } };
    return handlers;
}

error_t UpdateController::Handler(httpd_req_t *req) {
    char *buffer = new char[SCRATCH_BUFSIZE];

    auto controller = static_cast<UpdateController *>(req->user_ctx);

    auto res = controller->ReceiveFile(req, buffer);

    delete[] buffer;

    if (!res) {
        return STATUS_FAIL;
    }

    httpd_resp_send(req, "\n<<<<\n---Upload new firmware successfull---\n", HTTPD_RESP_USE_STRLEN);
    return STATUS_OK;
}

bool UpdateController::ReceiveFile(httpd_req_t *req, char *buffer) {
    log_i(TAG_UpdateController, "ReceiveFile, size: '%u'", (unsigned int)req->content_len);

    error_t err;
    const partition_t *update_partition;
    ota_handle_t update_handle;
    if (!BeginOta(&update_partition, &update_handle, req->content_len)) {
        return false;
    }

    int file_size = (int)req->content_len;
    while (file_size > 0) {
        int received = httpd_req_recv(req, buffer, MIN(SCRATCH_BUFSIZE, file_size));
        if (received < 0) {
            log_e(TAG_UpdateController, "ReceiveFile, error: '%d'", received);
            switch (received) {
                case HTTPD_SOCK_ERR_TIMEOUT:
                    httpd_resp_send_error_408(req);
                    break;

                case HTTPD_SOCK_ERR_INVALID:
                    httpd_resp_send_error_400(req);
                    break;

                default:
                    httpd_resp_send_error_500(req);
                    break;
            }
            return false;
        }
        file_size -= received;
        log_d(TAG_UpdateController, "ReceiveFile, received: %d, remained: %d", received, file_size);

        err = ota_write(update_handle, (const void *)buffer, received);
        if (err != STATUS_OK) {
            log_e(TAG_UpdateController, "Error: ota_write failed! err=0x%x", (unsigned int)err);
            httpd_resp_send_error_500(req);
            return false;
        }
    }

    if (!FinishOta(update_partition, &update_handle)) {
        httpd_resp_send_error_500(req);
        return false;
    }

    return true;
}

bool UpdateController::BeginOta(const partition_t **update_partition,
                                ota_handle_t *update_handle,
                                size_t image_size) {
    error_t err;
    const partition_t *configured = ota_get_boot_partition();
    const partition_t *running = ota_get_running_partition();

    if (configured == NULL) {
        log_e(TAG_UpdateController, "configured == NULL");
        return false;
    }
    if (running == NULL) {
        log_e(TAG_UpdateController, "running == NULL");
        return false;
    }

    if (configured != running) {
        log_w(TAG_UpdateController,
              "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
              (unsigned int)configured->address,
              (unsigned int)running->address);
        log_w(TAG_UpdateController,
              "(This can happen if either the OTA boot data or preferred boot image become "
              "corrupted somehow.)");
    }

    log_i(TAG_UpdateController,
          "Running partition type %d subtype %d (offset 0x%08x)",
          (int)running->type,
          (int)running->subtype,
          (unsigned int)running->address);

    *update_partition = ota_get_next_update_partition(NULL);

    if (*update_partition == NULL) {
        log_e(TAG_UpdateController, "update_partition == NULL");
        return false;
    }
    log_i(TAG_UpdateController,
          "Writing to partition subtype %u at offset 0x%x",
          (unsigned int)(*update_partition)->subtype,
          (unsigned int)(*update_partition)->address);

    err = ota_begin(*update_partition, image_size, update_handle);
    if (err != STATUS_OK) {
        log_e(TAG_UpdateController, "ota_begin failed, error=%d", err);
        return false;
    }

    log_i(TAG_UpdateController, "BeginOta succeeded");

    return true;
}

bool UpdateController::FinishOta(const partition_t *update_partition, ota_handle_t *update_handle) {
    error_t err;
    if (ota_end(*update_handle) != STATUS_OK) {
        log_e(TAG_UpdateController, "ota_end failed!");
        return false;
    }

    err = ota_set_boot_partition(update_partition);
    if (err != STATUS_OK) {
        log_e(TAG_UpdateController, "ota_set_boot_partition failed! err=0x%x", (unsigned int)err);
        return false;
    }
    log_i(TAG_UpdateController, "FinishOta succeeded");
    return true;
}