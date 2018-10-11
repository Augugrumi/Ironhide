//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_DBQUERY_H
#define IRONHIDE_DBQUERY_H

#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <curl/curl.h>

#include "jsoncpp.h"
#include "log.h"
#include "exceptions/failure.h"
#include "exceptions/endpoint_not_found.h"
#include "exceptions/query_ambiguous.h"
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

const char* const SI = "si";
} // namespace query

namespace reply {
const char* const RESULT = "result";
const char* const OK = "ok";
const char* const CONTENT = "content";
const char* const ERROR = "error";
const char* const REASON = "reason";
} // namespace result

const char* const ENDPOINT_PREFIX = "endpoints/";
const char* const ROUTE_PREFIX = "routes/";

enum endpoint_type{INGRESS_T, EGRESS_T};
enum protocol_type{TCP, UDP};

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
    ~DBQuery();

    class Endpoint {
    private:
        std::string ip;
        std::string socket_id;
        endpoint_type type_of_proxy;
    public:
        Endpoint(const std::string&,
                 const std::string&,
                 endpoint_type);
        std::string get_ip() const;
        std::string get_socket_id() const;
        endpoint_type get_endpoint_typology() const;
        std::string to_json() const;
    };

    class Query {
    private:
        const std::string item_id;
        const std::string ip_src;
        const std::string ip_dst;
        const uint16_t port_src;
        const uint16_t port_dst;
        const protocol_type prt;
        const std::string id_sfc;
        const std::vector<Endpoint> endpoints;
        Query(const std::string&,
              const std::string&,
              const std::string&,
              uint16_t,
              uint16_t,
              protocol_type,
              const std::string&,
              const std::vector<Endpoint>&);
    public:
        std::string get_item_id() const;
        std::string get_ip_src() const;
        std::string get_ip_dst() const;
        uint16_t get_port_src() const;
        uint16_t get_port_dst() const;
        protocol_type get_protocol_type() const;
        std::string get_id_sfc() const;
        std::vector<Endpoint> get_all_endpoints() const;
        Endpoint get_endpoint(endpoint_type) const;
        std::string to_json() const;
        std::string to_url() const;

        class Builder {
        private:
            std::string item_id;
            std::string ip_src;
            std::string ip_dst;
            uint16_t port_src;
            uint16_t port_dst;
            protocol_type prt;
            std::string id_sfc;
            std::vector<Endpoint> endpoints;
        public:
            Builder() = default;
            Builder& set_item_id(const std::string&);
            Builder& set_ip_src(const std::string&);
            Builder& set_ip_dst(const std::string&);
            Builder& set_port_src(uint16_t);
            Builder& set_port_dst(uint16_t);
            Builder& set_protocol(protocol_type);
            Builder& set_id_sfc(const std::string&);
            Builder& set_endpoint(const Endpoint&);
            Query build() const;
        };

        friend Query Query::Builder::build() const;
    };

    typedef Query Entry;

    std::string create_entry(const Query&);
    Entry get_entry(const char* id);
    bool delete_entry(const char* id);
    bool update_endpoint(const Query&, const Endpoint&);

    std::vector<utils::Address> get_route_list(uint32_t);
};

}

#endif //IRONHIDE_DBQUERY_H
