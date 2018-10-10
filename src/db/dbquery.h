//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_DBQUERY_H
#define IRONHIDE_DBQUERY_H

#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <curl/curl.h>
//#include <boost/log/trivial.hpp>

#include "jsoncpp.h"
#include "log.h"
#include "exceptions/failure.h"
#include "utils/address.h"
#include "utils/urlbuilder.h"

namespace db{

namespace query {
const char* const SRC_IP = "ipSrc";
const char* const DST_IP = "ipDst";
const char* const SRC_PORT = "portSrc";
const char* const DST_PORT = "portDst";
const char* const SFC_ID = "idSfc";
const char* const PROTOCOL = "protocol";
const char* const SOCK_EGRESS = "socketIdEgress";
const char* const SOCK_INGRESS = "socketIdIngress";
const char* const EGRESS_IP = "ipEgress";
const char* const INGRESS_IP = "ipIngress";
const char* const INGRESS = "ingress";
const char* const EGRESS = "egress";
} // namespace query

namespace reply {
const char* const RESULT = "result";
const char* const OK = "ok";
const char* const CONTENT = "content";
const char* const ERROR = "error";
const char* const REASON = "reason";
} // namespace result

const char* const ENDPOINT_PREFIX = "endpoints/";

enum endpoint_type{INGRESS_T, EGRESS_T};
enum protocol_type{TCP, UDP};

// TODO copy get next from astaire
// TODO make requests to roulette really
class DBQuery {
private:
    const utils::Address roulette_addr;
    CURL* curl = nullptr;

    static size_t curl_callback(void*, size_t, size_t, std::string*);
    bool is_op_ok(const std::string&);
    bool handle_req(const CURLcode&, std::function<bool()>);
public:
    DBQuery(const std::string&, uint16_t port);
    DBQuery(const utils::Address&);

    std::string create_entry(const char* ip_src,
                      const char* ip_dst,
                      uint16_t port_src,
                      uint16_t port_dst,
                      protocol_type protocol,
                      const char* id_sfc,
                      endpoint_type endpoint,
                      const char* endpoint_ip,
                      const char* endpoint_socket);
    bool update_endpoint(const char* ip_src,
                         const char* ip_dst,
                         uint16_t port_src,
                         uint16_t port_dst,
                         protocol_type protocol,
                         const char* id_sfc,
                         endpoint_type endpoint,
                         const char* endpoint_ip,
                         const char* endpoint_socket,
                         const char* new_ip,
                         const char* new_socked_id);

    bool delete_endpoint(const char* id);

    // TODO get_next_route, get_chain_route, get_endpoint

    ~DBQuery();
};

}

#endif //IRONHIDE_DBQUERY_H
