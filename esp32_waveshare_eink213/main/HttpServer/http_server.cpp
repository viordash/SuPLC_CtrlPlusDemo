
#include "http_server.h"
#include "DisplayController.h"
#include "HttpServer.h"
#include "InputController.h"
#include "MainController.h"
#include "UpdateController.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool http_server_started = false;
MainController *mainController;
UpdateController *updateController;
DisplayController *displayController;
InputController *inputController;
HttpServer *httpServer;

void start_http_server(RenderingService &rendering_service) {
    if (http_server_started) {
        stop_http_server();
        return;
    }
    mainController = new MainController();
    updateController = new UpdateController();
    displayController = new DisplayController(rendering_service);
    inputController = new InputController;
    httpServer =
        new HttpServer({ updateController, mainController, displayController, inputController });
    httpServer->Start();
    http_server_started = true;
}

void stop_http_server() {
    if (!http_server_started) {
        return;
    }
    httpServer->Stop();
    delete inputController;
    delete displayController;
    delete updateController;
    delete mainController;
    delete httpServer;
    http_server_started = false;
}