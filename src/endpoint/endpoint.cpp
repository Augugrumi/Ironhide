//
// Created by zanna on 05/10/18.
//

#include "endpoint.h"

classifier::Classifier endpoint::Endpoint::classifier_;
db::DBQuery endpoint::Endpoint::roulette_("localhost", 57684);

void endpoint::Endpoint::add_entry(endpoint::ConnectionEntry ce,
                                   endpoint::socket_fd fd,
                                   db::endpoint_type endpoint,
                                   db::protocol_type protocol) {
    connection_map[ce] = fd;
    std::string response = roulette_.create_entry(
            db::DBQuery::Query::Builder()
                   .set_id_sfc(ce.get_sfcid())
                   .set_protocol(protocol)
                   .set_endpoint(db::DBQuery::Endpoint(
                           get_my_ip(), std::to_string(fd), endpoint))
                   .set_ip_src(ce.get_ip_src())
                   .set_ip_dst(ce.get_ip_dst())
                   .set_port_src(ce.get_port_src())
                   .set_port_dst(ce.get_port_dst())
                   .build());
    if (response != "")
        map_to_remote[ce] = response;
}

void
endpoint::Endpoint::update_entry(endpoint::ConnectionEntry ce,
                                 endpoint::socket_fd fd,
                                 db::endpoint_type endpoint) {
    roulette_.update_endpoint(
            db::DBQuery::Query::Builder()
                    .set_id_sfc(ce.get_sfcid())
                    .set_ip_src(ce.get_ip_src())
                    .set_ip_dst(ce.get_ip_dst())
                    .set_port_src(ce.get_port_src())
                    .set_port_dst(ce.get_port_dst())
                    .build(),
            db::DBQuery::Endpoint(
                    get_my_ip(), std::to_string(fd), endpoint)
            );
    connection_map[ce] = fd;
}

void endpoint::Endpoint::delete_entry(endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it != connection_map.end()) {
        connection_map.erase(ce);
        roulette_.delete_entry(map_to_remote[ce].c_str());
        map_to_remote.erase(ce);
    }
}

endpoint::socket_fd endpoint::Endpoint::retrieve_connection(
        endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it == connection_map.end()) {
        return -1;
    }
    return it->second;
}

void endpoint::Endpoint::set_my_ip(const std::string &my_ip) {
    my_ip_ = my_ip;
}

std::string endpoint::Endpoint::get_my_ip() const {
    return my_ip_;
}
