#pragma once

#include "BaseController.h"
#include "board.h"
#include <vector>

class HttpServer {
  public:
    explicit HttpServer(std::vector<BaseController *> const &controllers);
    ~HttpServer();

    bool Start();
    void Stop();

  private:
    std::vector<BaseController *> controllers;
    httpd_handle_t server = NULL;
};
