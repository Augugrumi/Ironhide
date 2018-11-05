#include <iostream>
#include <thread>

#include "endpoint/ingress.h"
#include "endpoint/egress.h"

#include "config.h"

/**
 * Help printing function
 */
void usage() {
    const char message[] =
            "\n"
            "Example of usage:\n"
            "\t ./ironhide [OPTIONS]\n"
            " -e   To set the egress mode - Default ingress mode\n"
            " -i   To set the port used for internal connections - Default 8778\n"
            " -o   To set the external port used - Default 8787\n"
            " -h   Show this message\n"
            "'ROULETTE_SERVICE' can be set into the env to change remote address";
    std::cout << message << std::endl;
}

int main(int argc, char *argv[]) {
#if DEBUG_BUILD
    utils::Log::instance()->set_log_level(utils::Log::trace);
#endif
    uint16_t ext_port = 8787;
    uint16_t int_port = 8778;
    bool is_egress = false;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ei:o:h")) != -1) {
        switch (c) {
            case 'e':
                is_egress = true;
                break;
            case 'i':
                int_port = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'o':
                ext_port = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'h':
                usage();
                exit(0);
            default:
                std::cout << "Unrecognized option" << std::endl;
                usage();
                exit(1);
        }
    }

    if (const char *env_p = std::getenv("ROULETTE_SERVICE")) {
        endpoint::Endpoint::set_remote(env_p);
    }

    if (is_egress) {
        LOG(linfo, "Starting as egress...");
        endpoint::Egress e(ext_port, int_port);
        e.start();
    } else {
        LOG(linfo, "Starting as ingress...");
        endpoint::Ingress i(ext_port, int_port);
        i.start();
    }

    return 0;
}
