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

    utils::Log::instance()->set_log_level(utils::Log::Level::debug);

    db::DBQuery query("localhost", 57684);

    std::string result = query.create_entry(
                db::DBQuery::Query::Builder()
                .set_ip_src("123.123.123.123")
                .set_ip_dst("222.222.222.222")
                .set_port_src(80)
                .set_port_dst(80)
                .set_protocol(db::protocol_type::TCP)
                .set_id_sfc("bla")
                .set_endpoint(db::DBQuery::Endpoint(
                                  "42.42.42.42",
                                  "456",
                                  db::endpoint_type::INGRESS_T))
                .build()
                );

    LOG(linfo, "1 - Operation succeded? " + result);
    /*bool result1 = query.update_endpoint("123.123.123.123", "222.222.222.222", 80, 80, db::protocol_type::TCP, "bla", db::endpoint_type::INGRESS_T, "42.42.42.42", "456", "255.255.255.255", "69");*/
    bool result1 = query.update_endpoint(db::DBQuery::Query::Builder()
                                         .set_ip_src("123.123.123.123")
                                         .set_ip_dst("222.222.222.222")
                                         .set_port_src(80)
                                         .set_port_dst(80)
                                         .set_protocol(db::protocol_type::TCP)
                                         .set_id_sfc("bla")
                                         .set_endpoint(db::DBQuery::Endpoint(
                                                           "42.42.42.42",
                                                           "456",
                                                           db::endpoint_type::INGRESS_T))
                                         .build(),
                                         db::DBQuery::Endpoint(
                                             "255.255.255.255",
                                             "69",
                                             db::endpoint_type::EGRESS_T));
    LOG(linfo, "2 - Operation succeded? " + std::to_string(result1));
    /*result1 = query.delete_endpoint(result.c_str());
    LOG(linfo, "3 - Operation succeded? " + std::to_string(result1));*/
}
