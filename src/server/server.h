#ifndef IRONHIDE_SERVER_H
#define IRONHIDE_SERVER_H

#include "config.h"

#include <cstdint>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <cstdio>
#include <functional>

#include "log.h"

#define BACKLOG 10

namespace server {

typedef std::function<void(void*)> pkt_mngmnt_lambda;

class Server {
protected:
    /**
     * port used by the server
     */
    const uint16_t port_;
    /**
     * to stop the server
     */
    bool stopped_;
    /**
     * lambda function to manage entering packets
     */
    pkt_mngmnt_lambda manager_;

    void setup_sign_catching();
    static void signal_handler(int signal_number);
public:
    /**
     * Constructor
     * @param port Port on which the udp server waits for packets
     */
    explicit Server(uint16_t port);
    /**
     * Start server
     */
    virtual void run() = 0;
    /**
     * To set the function to manage entering packets
     * @param pkt_mngmnt_lambda Function used to manage packets
     */
    virtual void set_pkt_manager(pkt_mngmnt_lambda);
    /**
     * To stop the server
     */
    void stop();

    virtual ~Server() = default;
};

}
#endif //IRONHIDE_SERVER_H
