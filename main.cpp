#include <iostream>
#include <thread>

#include "src/servertcp.h"
#include "src/serverudp.h"

void usage() {
    const char message[] =
            "\n"
            "Example of usage:\n"
            "\t ./ironhide [OPTIONS]\n"
            " -e   To set the egress mode - Default ingress mode\n"
            " -i   To set the port used for internal connections - Default 8778\n"
            " -o   To set the external port used - Default 8787\n"
            " -h   Show this message\n";
    std::cout <<message<<std::endl;
}

int main(int argc, char* argv[]) {
    uint16_t ext_port = 8787;
    uint16_t int_port = 8778;
    bool is_egress = false;

    int c;
    opterr = 0;
    while ((c = getopt(argc, (char **)argv, "ei:o:h")) != -1) {
        switch(c) {
            case 'e':
                is_egress = true;
                break;
            case 'i':
                int_port = atoi(optarg);
                break;
            case 'o':
                ext_port = atoi(optarg);
                break;
            case 'h':
                usage();
                exit(0);
        }
    }

    std::thread udp_internal_thread([&int_port]{
        (new server::udp::ServerUDP(int_port))->run();
    });

    if (is_egress) {
        // TODO implement logic for egress
        udp_internal_thread.join();
    } else {

        std::thread tcp_thread([&ext_port]{
            (new server::tcp::ServerTCP(ext_port))->run();
        });
        std::thread udp_external_thread([&ext_port]{
            (new server::udp::ServerUDP(ext_port))->run();
        });

        udp_internal_thread.join();
        tcp_thread.join();
        udp_external_thread.join();
        std::cout << "Hello, World!" << std::endl;
    }
    return 0;
}