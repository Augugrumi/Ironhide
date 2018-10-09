#include <iostream>
#include <thread>

#include "endpoint/ingress.h"
#include "endpoint/egress.h"

#include "db/dbquery.h"
#include "log.h"

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
    /*uint16_t ext_port = 8787;
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
        endpoint::Egress e;
        e.start(int_port, ext_port);
    } else {
        endpoint::Ingress i;
        i.start(int_port, ext_port);
    }

    return 0;*/

    utils::Log::instance()->set_log_level(utils::Log::Level::trace);

    db::DBQuery query("localhost", 57684);
    bool result = query.create_entry("123.123.123.123", "222.222.222.222", 80, 80, db::protocol_type::TCP, "bla", db::endpoint_type::INGRESS_T, "42.42.42.42", "456");
    LOG(linfo, "1 - Operation succeded? " + std::to_string(result));
    result = query.update_endpoint("123.123.123.123", "222.222.222.222", 80, 80, db::protocol_type::TCP, "bla", db::endpoint_type::INGRESS_T, "42.42.42.42", "456", "255.255.255.255", "69");
    LOG(linfo, "2 - Operation succeded? " + std::to_string(result));
}
