//
// Created by zanna on 04/10/18.
//

#include <cstdlib>
#include "server.h"

server::Server::Server(uint16_t port) : port_(port){}

void server::Server::stop() {
    stopped_ = true;
}

void server::Server::signal_handler(int signal_number) {
    /* TODO: Put exit cleanup code here. */
    exit(0);
}