//
// Created by zanna on 05/10/18.
//

#include "endpoint.h"

classifier::Classifier endpoint::Endpoint::classifier_;
// TODO retrieve roulette address
db::DBQuery* endpoint::Endpoint::roulette_ = new db::DBQuery("localhost", 57684);

endpoint::Endpoint::Endpoint() {
    retrieve_ip();
}

void endpoint::Endpoint::add_entry(endpoint::ConnectionEntry ce,
                                   endpoint::socket_fd fd,
                                   db::endpoint_type endpoint,
                                   db::protocol_type protocol) {
    LOG(ltrace, get_my_ip());
    connection_map[ce] = fd;
    std::string response = roulette_->create_entry(
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
    LOG(ltrace, response);
    if (response != "")
        map_to_remote[ce] = response;
}

void endpoint::Endpoint::update_entry(endpoint::ConnectionEntry ce,
                                      endpoint::socket_fd fd,
                                      db::endpoint_type endpoint) {
    LOG(ldebug, "here1");

    auto s = db::DBQuery::Query::Builder()
            .set_id_sfc(ce.get_sfcid())
            .set_ip_src(ce.get_ip_src())
            .set_ip_dst(ce.get_ip_dst())
            .set_port_src(ce.get_port_src())
            .set_port_dst(ce.get_port_dst())
            .build();

    LOG(ldebug, s.to_url());

    roulette_->update_endpoint(
            db::DBQuery::Query::Builder()
                    .set_endpoint_type(db::endpoint_type::INGRESS_T)
                    .set_id_sfc(ce.get_sfcid())
                    .set_ip_src(ce.get_ip_src())
                    .set_ip_dst(ce.get_ip_dst())
                    .set_port_src(ce.get_port_src())
                    .set_port_dst(ce.get_port_dst())
                    .build(),
            db::DBQuery::Endpoint(
                    get_my_ip(), std::to_string(fd), endpoint)
            );
    LOG(ldebug, "here2");
    connection_map[ce] = fd;
}

void endpoint::Endpoint::delete_entry(endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it != connection_map.end()) {
        connection_map.erase(ce);
        roulette_->delete_entry(map_to_remote[ce].c_str());
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

void endpoint::Endpoint::retrieve_ip() {
    const char * interface = "eth0";
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(interface, ifa->ifa_name) == 0) {
                set_my_ip(addressBuffer);
            }
            //printf("IP Address %s\n", ifa->ifa_name, addressBuffer);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

}

std::string endpoint::Endpoint::get_my_ip() const {
    return my_ip_;
}

// TODO to refactor
void endpoint::Endpoint::set_remote(const char *ip, uint16_t port) {
    delete(roulette_);
    roulette_ = new db::DBQuery(ip, port);
}
