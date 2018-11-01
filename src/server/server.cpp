//
// Created by zanna on 04/10/18.
//
#include "server.h"

server::Server::Server(uint16_t port) : port_(port){}

void server::Server::stop() {
    stopped_ = true;
}

void server::Server::signal_handler(int signal_number) {
    /* TODO: Put exit cleanup code here. */
    LOG(lwarn, std::string("Interruption, code: ") +
               std::to_string(signal_number));
    exit(0);
}

void server::Server::setup_sign_catching() {
    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, Server::signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, Server::signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
}

void server::Server::set_pkt_manager(server::pkt_mngmnt_lambda manager) {
    manager_ = std::move(manager);
}
