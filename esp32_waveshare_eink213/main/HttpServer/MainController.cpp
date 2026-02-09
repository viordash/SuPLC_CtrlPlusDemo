#include "MainController.h"
#include "UriHelper.h"
#include "os.h"
#include <algorithm>
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG_MainController = "main_controller";

struct WebFile {
    const uint8_t *data;
    const size_t data_size;
    const char *name;
};

#define EMBEDDED_FILE_SYMBOL(symbol, base_name) EMBEDDED_FILE_SYMBOL_IMPL(symbol, base_name)
#define EMBEDDED_FILE_SYMBOL_IMPL(symbol, base_name)                                               \
    extern uint8_t _binary_##symbol##_start[];                                                     \
    extern uint8_t _binary_##symbol##_end;                                                         \
    static const WebFile base_name##_file = { .data = _binary_##symbol##_start,                    \
                                              .data_size = (size_t)(&_binary_##symbol##_end        \
                                                                    - _binary_##symbol##_start),   \
                                              .name = base_name##_name };

#define URI_HANDLER(file)                                                                          \
    { .uri = "/" file##_name,                                                                      \
      .method = HTTP_GET,                                                                          \
      .handler = MainController::SendingFile,                                                      \
      .user_ctx = (void *)&file##_file }

// EMBEDDED_FILE_SYMBOL(index_html_symbol, index_html)
// EMBEDDED_FILE_SYMBOL(favicon_ico_symbol, favicon_ico)
// EMBEDDED_FILE_SYMBOL(main_js_symbol, main_js)
// EMBEDDED_FILE_SYMBOL(styles_css_symbol, styles_css)

MainController::MainController() : BaseController() {
}

MainController::~MainController() {
}

std::vector<httpd_uri_t> MainController::GetUriHandlers() {
    std::vector<httpd_uri_t> handlers = { //
                                          {
                                              .uri = "/",
                                              .method = HTTP_GET,
                                              .handler = MainController::SendingFile,
                                            //   .user_ctx = (void *)&index_html_file //
                                          },
                                          {
                                              .uri = "/main",
                                              .method = HTTP_GET,
                                              .handler = MainController::SendingFile,
                                            //   .user_ctx = (void *)&index_html_file //
                                          },
                                        //   URI_HANDLER(index_html),
                                        //   URI_HANDLER(favicon_ico),
                                        //   URI_HANDLER(main_js),
                                        //   URI_HANDLER(styles_css)
    };

    return handlers;
}

error_t MainController::SendingFile(httpd_req_t *req) {
    auto web_file = static_cast<WebFile *>(req->user_ctx);

    const uint8_t *data = web_file->data;
    size_t data_size = web_file->data_size;
    const char *filename = web_file->name;
    size_t pos = 0;

    log_i(TAG_MainController, "Sending file : %s (%u bytes)...", filename, (unsigned)data_size);
    error_t res = httpd_resp_set_type(req, UriHelper::GetContentType(filename));
    if (res != STATUS_OK) {
        return res;
    }

    res = httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600");
    if (res != STATUS_OK) {
        return res;
    }

    char *chunk = new char[SCRATCH_BUFSIZE];
    do {
        size_t chunk_size = std::min(data_size, (size_t)SCRATCH_BUFSIZE);
        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, (const char *)&data[pos], chunk_size) != STATUS_OK) {
            log_e(TAG_MainController, "File sending failed!");
            /* Abort sending file */
            httpd_resp_send_chunk(req, NULL, 0);

            /* Respond with 500 Internal Server Error */
            httpd_resp_send_error_500(req);
            break;
        }
        pos += chunk_size;
        data_size -= chunk_size;
    } while (data_size > 0);
    delete[] chunk;

    if (data_size != 0) {
        return STATUS_FAIL;
    }
    /* Respond with an empty chunk to signal HTTP response completion */
    return httpd_resp_send_chunk(req, NULL, 0);
}