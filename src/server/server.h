//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_SERVER_H
#define IRONHIDE_SERVER_H

#include "config.h"

#include <cstdint>
#include <cstdlib>
#include <csignal>
#include <errno.h>
#include <stdio.h>
#include <functional>

#define BACKLOG 10

namespace server {

typedef std::function<void(void*)> pkt_mngmnt_lambda;

class Server {
protected:
    const uint16_t port_;
    bool stopped_;
    pkt_mngmnt_lambda manager_;

    void setup_sign_catching();
    static void signal_handler(int signal_number);
public:
    Server(uint16_t port);
    virtual void run() = 0;
    virtual void set_pkt_manager(pkt_mngmnt_lambda);
    void stop();

    virtual ~Server() = default;
};

}
#endif //IRONHIDE_SERVER_H
