#include <iostream>
#include <thread>

#include "src/servertcp.h"
#include "src/serverudp.h"

int main() {
    std::thread tcp_thread([]{
        (new server::tcp::ServerTCP(8787))->run();
    });
    std::thread udp_thread([]{
        (new server::udp::ServerUDP(8787))->run();
    });
    udp_thread.join();
    tcp_thread.join();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}