//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_SERVER_H
#define IRONHIDE_SERVER_H

#include <cstdint>

#define BACKLOG 10
#define BUFFER_SIZE 8196

namespace server {

class Server {
protected:
    const uint16_t port_;
    bool stopped_;

    virtual void pkt_mngmnt(void*) = 0;
    static void signal_handler(int signal_number);
public:
    Server(uint16_t port);
    virtual void run() = 0;
    void stop();

    virtual ~Server() = default;
};

}
#endif //IRONHIDE_SERVER_H
