//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_ENDPOINT_H
#define IRONHIDE_ENDPOINT_H

#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <map>

#include "connectionentry.h"
#include "../classifier/classifier.h"
#include "../db/dbquery.h"
#include "log.h"

namespace endpoint {

typedef int socket_fd;
typedef std::pair<socket_fd, sockaddr_in> sock_conn_t;
typedef std::pair<ConnectionEntry, sock_conn_t> conn_map_entry_t;

/**
 * Class on top of hierarchy for ingress and egress
 */
class Endpoint {
private:
    /**
     * Map to maintain data of connections
     */
    std::map<ConnectionEntry, std::pair<socket_fd, sockaddr_in>> connection_map;
    /**
     * Map to maintain the id used on roulette
     */
    std::map<ConnectionEntry, std::string> map_to_remote;
    /**
     * IP of the machine
     */
    std::string my_ip_;
    /**
     * Port exposed to the external
     */
    uint16_t ext_port_;
    /**
     * Port exposed to the chain
     */
    uint16_t int_port_;
protected:
    /**
     * Object that classify the packets
     * TODO modify has-a relation, remote obj?
     */
    static classifier::Classifier classifier_;
    /**
     * To perform queries to roulette
     */
    static db::DBQuery* roulette_;
    /**
     * To add a new connection to local map and to remote roulette
     * @param ConnectionEntry ce information on addresses
     * @param socket_fd socket used for connection
     * @param sockaddr_in data used by the connection
     * @param db::endpoint_type ingress or egress
     * @param db::protocol_type connection protocol
     */
    void add_entry(ConnectionEntry, socket_fd, sockaddr_in,
                   db::endpoint_type, db::protocol_type);
    /**
     * Update local map and to remote roulette
     * @param ConnectionEntry ce information on addresses
     * @param socket_fd socket used for connection
     * @param sockaddr_in data used by the connection
     * @param db::endpoint_type ingress or egress
     */
    void update_entry(ConnectionEntry, endpoint::socket_fd, sockaddr_in,
                      db::endpoint_type);
    /**
     * To delete a connection to local map and to remote roulette
     * @param ConnectionEntry ce information on addresses
     */
    void delete_entry(ConnectionEntry); // even by socket?
    /**
     * To retrieve information on a connection
     * @param ConnectionEntry ce information on addresses
     * @return data on the connection
     */
    std::pair<socket_fd, sockaddr_in> retrieve_connection_2(ConnectionEntry);
    /**
     * To access the internal port
     * @return internal port
     */
    uint16_t get_internal_port() const;
    /**
     * To access the external port
     * @return external port
     */
    uint16_t get_external_port() const;

    // TODO set it somewhere -> passing as argument to main so it easier to dockerize?
    /**
     * To set the IP of the machine
     * @param my_ip string that represent the machine IPv4 with dotted notation
     */
    void set_my_ip(const std::string& my_ip);
    /**
     * To access the IP set
     * @return the IP set
     */
    std::string get_my_ip() const;
    /**
     * Retrieve the IP used by the machine on interface eth0*
     * (it could have a suffix after 'eth0' as in docker) and set it on my_ip
     * member
     */
    void retrieve_ip();
public:
    /**
     * Constructor
     * @param ext_port Port exposed to the external
     * @param int_port Port exposed to the chain
     */
    Endpoint(uint16_t ext_port, uint16_t int_port);
    /**
     * To start the endpoint
     */
    virtual void start() = 0;
    /**
     * To set the address of roulette
     * @param ip address of roulette in the format <ip>:<port>
     */
    static void set_remote(const char* ip);
};

} // namespace endpoint

#endif //IRONHIDE_ENDPOINT_H
