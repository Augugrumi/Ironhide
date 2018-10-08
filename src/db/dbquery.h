//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_DBQUERY_H
#define IRONHIDE_DBQUERY_H

#include <string>
#include <memory>
#include <functional>
#include <curl/curl.h>
#include <boost/log/trivial.hpp>

#include "jsoncpp.h"
#include "address.h"
#include "exceptions/failure.h"

namespace db{

namespace query {
const char* SRC_IP = "ipSrc";
const char* DST_IP = "ipDst";
const char* SRC_PORT = "portSrc";
const char* DST_PORT = "portDst";
const char* SFC_ID = "idSfc";
const char* PROTOCOL = "protocol";
const char* SOCK_EGRESS = "socketIdEgress";
const char* SOCK_INGRESS = "socketIdIngress";
const char* EGRESS_IP = "ipEgress";
const char* INGRESS_IP = "ipIngress";
const char* INGRESS = "ingress";
const char* EGRESS = "egress";
} // namespace query

namespace reply {
const char* RESULT = "result";
const char* OK = "ok";
const char* CONTENT = "content";
const char* ERROR = "error";
const char* REASON = "reason";
} // namespace result

const char* ENDPOINT_PREFIX = "endpoints/";

enum endpoint_type{INGRESS_T, EGRESS_T};
enum protocol_type{TCP, UDP};

// TODO copy get next from astaire
// TODO make requests to roulette really
class DBQuery {
private:
    const Address roulette_addr;
    CURL* curl = nullptr;

    static size_t curl_callback(void*, size_t, size_t, std::string*);
public:
    DBQuery(const std::string&, uint16_t port);
    DBQuery(const Address&);

    bool create_entry(char* ip_src, char* ip_dst,
                      uint16_t port_src, uint16_t port_dst,
                      protocol_type protocol,
                      char* id_sfc,
                      endpoint_type endpoint,
                      char* endpoint_ip, int endpoint_socket);
    Address update_endpoint(char* ip_src, char* ip_dst,
                         uint16_t port_src, uint16_t port_dst,
                         protocol_type protocol,
                         char* id_sfc,
                         endpoint_type endpoint,
                         char* endpoint_ip, int endpoint_socket);
    ~DBQuery();
};

}

#endif //IRONHIDE_DBQUERY_H
