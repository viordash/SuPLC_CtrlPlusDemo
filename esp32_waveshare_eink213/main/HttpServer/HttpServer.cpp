
#include "HttpServer.h"
#include "os.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *TAG_HttpServer = "http_server";

HttpServer::HttpServer(std::vector<BaseController *> const &controllers) {
    this->controllers = controllers;
}

HttpServer::~HttpServer() {
    Stop();
}

void close_func(httpd_handle_t hd, int sockfd) {
    (void)hd;
    shutdown(sockfd, SHUT_WR);
    close(sockfd);
}

bool HttpServer::Start() {
    httpd_config_t config = HTTPD_CONFIG_DEFAULT();

    if (httpd_start(&server, &config) != STATUS_OK) {
        log_e(TAG_HttpServer, "Start error");
        server = NULL;
        return false;
    }
    log_i(TAG_HttpServer, "Starting, listen port:%u", config.server_port);

    for (const auto &controller : controllers) {
        for (const auto &uriHandler : controller->GetUriHandlers()) {
            error_t res = httpd_register_uri_handler(server, &uriHandler);
            if (res == STATUS_OK) {
                log_i(TAG_HttpServer, "reg URI:%s", uriHandler.uri);
            } else {
                log_e(TAG_HttpServer, "reg URI:%s, error:0x%04X", uriHandler.uri, res);
            }
        }
    }

    return true;
}

void HttpServer::Stop() {
    if (server != NULL) {
        httpd_stop(server);
        log_i(TAG_HttpServer, "Stopped");
    }
    server = NULL;
}
