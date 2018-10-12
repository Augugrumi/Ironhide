#include <iostream>
#include <thread>

#include "endpoint/ingress.h"
#include "endpoint/egress.h"

#include "config.h"
#include "db/dbquery.h"
#include "log.h"

// TODO Remove me - only for test purposes
#include <vector>

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
#if DEBUG_BUILD
    utils::Log::instance()->set_log_level(utils::Log::trace);
#endif
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

    if (is_egress) {
        LOG(linfo, "Starting as egress...");
        endpoint::Egress e;
        e.start(int_port, ext_port);
    } else {
        LOG(linfo, "Starting as ingress...");
        endpoint::Ingress i;
        i.start(int_port, ext_port);
    }

    return 0;
}
