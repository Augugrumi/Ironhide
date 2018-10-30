//
// Created by zanna on 05/10/18.
//

#include "endpoint.h"

classifier::Classifier endpoint::Endpoint::classifier_;
// TODO retrieve roulette address
db::DBQuery* endpoint::Endpoint::roulette_ = new db::DBQuery("localhost", 57684);



endpoint::Endpoint::Endpoint(uint16_t ext_port, uint16_t int_port) :
        ext_port_(ext_port), int_port_(int_port){
    retrieve_ip();
    //my_ip_ = "127.0.0.1";
}

uint16_t endpoint::Endpoint::get_internal_port() const {
    return int_port_;
}

uint16_t endpoint::Endpoint::get_external_port() const {
    return ext_port_;
}

void endpoint::Endpoint::add_entry(endpoint::ConnectionEntry ce,
                                   endpoint::socket_fd fd,
                                   sockaddr_in sockin,
                                   db::endpoint_type endpoint,
                                   db::protocol_type protocol) {
    LOG(ldebug, "Adding entry");
    LOG(ltrace, get_my_ip());
    connection_map.insert(std::pair<ConnectionEntry,
            std::pair<socket_fd, sockaddr_in>>(ce,
                    std::pair<socket_fd, sockaddr_in>(fd,sockin)));
    std::string response = roulette_->create_entry(
            db::DBQuery::Query::Builder()
                    .set_id_sfc(ce.get_sfcid())
                    .set_protocol(protocol)
                    .set_endpoint(db::DBQuery::Endpoint(
                            get_my_ip() + ":" + std::to_string(int_port_),
                            std::to_string(fd), endpoint))
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
                                      sockaddr_in sockin,
                                      db::endpoint_type endpoint) {
    roulette_->update_endpoint(
            db::DBQuery::Query::Builder()
                    .set_endpoint_type(db::endpoint_type::INGRESS_T)
                    .set_id_sfc(ce.get_sfcid())
                    .set_ip_src(ce.get_ip_src())
                    .set_ip_dst(ce.get_ip_dst())
                    .set_port_src(ce.get_port_src())
                    .set_port_dst(ce.get_port_dst())
                    .set_protocol(ce.get_protocol_type())
                    .build(),
            db::DBQuery::Endpoint(
                    get_my_ip() + ":" + std::to_string(int_port_),
                    std::to_string(fd), endpoint)
            );

    connection_map.insert(conn_map_entry_t(ce, sock_conn_t(fd,sockin)));
}

void endpoint::Endpoint::delete_entry(endpoint::ConnectionEntry ce) {
    LOG(ldebug, "Deleting...");
    auto it = connection_map.find(ce);
    auto itend = connection_map.end();
    if (it != itend) {
        connection_map.erase(ce);
        roulette_->delete_entry(map_to_remote[ce].c_str());
        map_to_remote.erase(ce);
    }
}

std::pair<endpoint::socket_fd, sockaddr_in>
        endpoint::Endpoint::retrieve_connection_2(endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it == connection_map.end()) {
        return sock_conn_t(-1, sockaddr_in());
    }
    return it->second;
}

void endpoint::Endpoint::set_my_ip(const std::string &my_ip) {
    my_ip_ = my_ip;
}

void endpoint::Endpoint::retrieve_ip() {
    const char * interface = "wlan0";
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
void endpoint::Endpoint::set_remote(const char *ip) {
    delete(roulette_);
    roulette_ = new db::DBQuery(db::utils::Address(ip));
}
