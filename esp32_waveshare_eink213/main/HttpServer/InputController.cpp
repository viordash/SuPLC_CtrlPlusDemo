#include "InputController.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "os.h"
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG_InputController = "InputController";

InputController::InputController() : BaseController() {
}

InputController::~InputController() {
}

std::vector<httpd_uri_t> InputController::GetUriHandlers() {
    std::vector<httpd_uri_t> handlers = { {
        .uri = "/keypress",
        .method = HTTP_POST,
        .handler = InputController::KeyPress,
        .user_ctx = this //
    } };
    return handlers;
}

error_t InputController::KeyPress(httpd_req_t *req) {
    char buffer[256];

    error_t ret = httpd_req_recv(req, buffer, sizeof(buffer));
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_error_408(req);
        }
        return STATUS_FAIL;
    }
    buffer[ret] = '\0';

    int key_code;
    int keydown;
    int parsed = sscanf(buffer, "{\"key\":%d,\"down\":%d}", &key_code, &keydown);
    if (parsed != 2) {
        httpd_resp_send_error_400(req);
        return STATUS_OK;
    }

    log_d(TAG_InputController, "KeyPress key: %d, down:%d", key_code, keydown);

    const int ArrowUp = 38;
    const int ArrowDown = 40;
    const int ShiftRight = 16;

    switch (key_code) {
        case ArrowUp:
            raise_event(Controller::gpio_events, keydown ? BUTTON_UP_IO_CLOSE : BUTTON_UP_IO_OPEN);
            break;
        case ArrowDown:
            raise_event(Controller::gpio_events,
                        keydown ? BUTTON_DOWN_IO_CLOSE : BUTTON_DOWN_IO_OPEN);
            break;
        case ShiftRight:
            raise_event(Controller::gpio_events,
                        keydown ? BUTTON_SELECT_IO_CLOSE : BUTTON_SELECT_IO_OPEN);
            break;

        default:
            break;
    }

    error_t res = httpd_resp_send(req, NULL, 0);
    if (res != STATUS_OK) {
        log_e(TAG_InputController, "Scroll httpd_resp_send error");
        return res;
    }

    return STATUS_OK;
}
