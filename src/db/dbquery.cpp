#include "dbquery.h"

// Static methods
std::string db::DBQuery::sanitize(const std::string &to_sanitize) {
    std::string res(to_sanitize);
    std::replace(res.begin(), res.end(), '\0', ' ');
    return res;
}

CURL *db::DBQuery::init_local_res() {
    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        perror("Impossible to properly init curl");
        exit(EXIT_FAILURE);
    }
    return curl;
}

void db::DBQuery::clear_local_res(CURL *curl) {
    curl_easy_cleanup(curl);
}

size_t db::DBQuery::curl_callback(void *ptr, size_t size,
                                  size_t nmemb, std::string *data) {
    data->append(reinterpret_cast<char *>(ptr), size * nmemb);
    return size * nmemb;
}

// End static methods

// DBQuery
db::DBQuery::DBQuery(const utils::Address &r_a) : roulette_addr(r_a) {
    curl_global_init(CURL_GLOBAL_ALL);
}

db::DBQuery::DBQuery(const std::string &address, uint16_t port)
        : db::DBQuery(utils::Address(address, port)) {}

std::string db::DBQuery::create_entry(const Query &query) {
    std::function<std::string(CURL *)> real_req = [this, query](CURL *curl) {
        struct curl_slist *header = nullptr;
        std::string req_data_res;
        std::string req_addr = utils::URLBuilder()
                .set_address(roulette_addr)
                .add_path(ENDPOINT_PREFIX)
                .build();
        std::string json_query = query.to_json();

        LOG(ldebug, "URL to send data: " + req_addr);
        LOG(ldebug, "Data to send: " + json_query);

        header = curl_slist_append(header, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         db::DBQuery::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);
        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_query.c_str());

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(header);
        if (handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res))) {

            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            Json::Value response;

            std::string errors;
            reader->parse(req_data_res.c_str(),
                          req_data_res.c_str() + req_data_res.size(),
                          &response,
                          &errors);


            return response[reply::CONTENT]["id"].asString();
        }

        return std::string();
    };

    return curl_req_handle(real_req);
}

bool db::DBQuery::update_endpoint(const Query &query, const Endpoint &to_add) {
    std::function<bool(CURL *)> real_req = [this, query, to_add](CURL *curl) {
        struct curl_slist *header = nullptr;
        std::string req_data_res;
        std::string q = query.to_url();
        std::string req_addr = utils::URLBuilder()
                .set_address(roulette_addr)
                .add_path(q)
                .build();

        std::string json_data = to_add.to_json();

        LOG(ldebug, "URL to send data: " + req_addr);
        LOG(ldebug, "Data to send: " + json_data);

        header = curl_slist_append(header, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         db::DBQuery::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);
        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

        CURLcode res = curl_easy_perform(curl);
        return handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res));
    };

    return curl_req_handle<bool>(real_req);
}

db::DBQuery::Entry db::DBQuery::get_entry(const char *id) {
    std::function<db::DBQuery::Entry(CURL *)> real_req = [this, id](
            CURL *curl) {
        const std::string req_addr = utils::URLBuilder()
                .set_address(roulette_addr)
                .add_path(ENDPOINT_PREFIX)
                .add_path(id)
                .build();
        std::string req_data_res;

        curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         db::DBQuery::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

        CURLcode res = curl_easy_perform(curl);
        if (handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res))) {

            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            Json::Value response;

            std::string errors;
            reader->parse(req_data_res.c_str(),
                          req_data_res.c_str() + req_data_res.size(),
                          &response,
                          &errors);

            Json::Value content = response[reply::CONTENT];

            protocol_type prt =
                    content[query::endpoint::PROTOCOL] == "tcp" ? TCP : UDP;
            Endpoint ingress(content[query::endpoint::INGRESS_IP].asString(),
                             content[query::endpoint::SOCK_INGRESS].asString(),
                             INGRESS_T);

            Endpoint egress(content[query::endpoint::EGRESS_IP].asString(),
                            content[query::endpoint::SOCK_EGRESS].asString(),
                            EGRESS_T);


            return db::DBQuery::Entry::Builder()
                    .set_item_id(content["_id"]["$oid"].asString())
                    .set_id_sfc(content[query::endpoint::SFC_ID].asString())
                    .set_ip_src(content[query::endpoint::SRC_IP].asString())
                    .set_ip_dst(content[query::endpoint::DST_IP].asString())
                    .set_port_src(content[query::endpoint::SRC_PORT].asUInt())
                    .set_port_dst(content[query::endpoint::DST_PORT].asUInt())
                    .set_protocol(prt)
                    .set_endpoint(ingress)
                    .set_endpoint(egress)
                    .build();
        } else {
            return db::DBQuery::Entry::Builder().build();
        }
    };

    return curl_req_handle<db::DBQuery::Entry>(real_req);

}

bool db::DBQuery::delete_entry(const char *id) {
    std::function<bool(CURL *)> real_req = [this, id](CURL *curl) {
        const std::string req_addr = utils::URLBuilder()
                .set_address(roulette_addr)
                .add_path(ENDPOINT_PREFIX)
                .add_path(id)
                .build();
        LOG(ldebug, req_addr);
        std::string req_data_res;


        curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         db::DBQuery::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

        CURLcode res = curl_easy_perform(curl);
        return handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res));
    };

    return curl_req_handle(real_req);
}


std::vector<db::utils::Address> db::DBQuery::get_route_list(uint32_t p_id) {
    std::function<std::vector<db::utils::Address>(
            CURL *)> real_req = [this, p_id](CURL *curl) {

        const std::string req_addr = utils::URLBuilder()
                .set_address(roulette_addr)
                .add_path(ROUTE_PREFIX)
                .add_path(std::to_string(p_id))
                .build();
        std::vector<utils::Address> routes;
        std::string req_data_res;

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         db::DBQuery::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

        CURLcode res = curl_easy_perform(curl);
        if (handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res))) {

            LOG(ltrace, "after if");
            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            Json::Value response;

            std::string errors;
            reader->parse(req_data_res.c_str(),
                          req_data_res.c_str() + req_data_res.size(),
                          &response,
                          &errors);

            Json::Value content = response[reply::CONTENT];
            Json::Value si_list = response[reply::CONTENT][query::route::SI];

            LOG(ltrace, "SI list size: " + std::to_string(si_list.size()));
            /*for (Json::Value::ArrayIndex i = 0; i != response.size(); i++) {
                LOG(ltrace, response[i]["port"].asString());
            }*/
            for (const Json::Value &address : content[query::route::SI]) {
                utils::Address to_add(address[query::route::ADDRESS].asString(),
                                      address[query::route::PORT].asUInt());
                routes.push_back(to_add);
            }
            return routes;
        }
        return std::vector<utils::Address>();
    };

    return curl_req_handle(real_req);
}

bool db::DBQuery::handle_req(const CURLcode &res, std::function<bool()> cb) {
    if (res != CURLE_OK) {
        LOG(lwarn, std::string("curl_easy_perform() failed: ") +
                   std::string(curl_easy_strerror(res)));
        throw db::exceptions::ios_base::failure("Error while making the request"
                                                "to route backend");
    } else {
        return cb();
    }
}

bool db::DBQuery::is_op_ok(const std::string &req) {
    LOG(ldebug, "Request performed successfully");

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value response;

    std::string errors;
    LOG(ldebug, "Data obtained: " + req);
    bool read = reader->parse(req.c_str(),
                              req.c_str() + req.size(),
                              &response,
                              &errors);

    if (read) { // Read perform successfully
        return response[reply::RESULT] == reply::OK;
    } else {
        throw db::exceptions::ios_base::failure("Error while parsing the JSON"
                                                " reply");
    }
}

db::DBQuery::~DBQuery() {
    curl_global_cleanup();
}
// End DBQuery

// Endpoint
db::DBQuery::Endpoint::Endpoint(const std::string &new_ip,
                                const std::string &new_socket_id,
                                endpoint_type end_type)
        : ip(new_ip), socket_id(new_socket_id), type_of_proxy(end_type) {
}

std::string db::DBQuery::Endpoint::get_ip() const {
    return ip;
}

std::string db::DBQuery::Endpoint::get_socket_id() const {
    return socket_id;
}

db::endpoint_type db::DBQuery::Endpoint::get_endpoint_typology() const {
    return type_of_proxy;
}

std::string db::DBQuery::Endpoint::to_json() const {
    Json::Value json;

    if (type_of_proxy == INGRESS_T) {
        json[query::endpoint::INGRESS_IP] = ip;
        json[query::endpoint::SOCK_INGRESS] = socket_id;
    } else {
        json[query::endpoint::EGRESS_IP] = ip;
        json[query::endpoint::SOCK_EGRESS] = socket_id;
    }
    return sanitize(json.toStyledString());
}
// End Endpoint

// Query
db::DBQuery::Query::Query(const std::string &new_item_id,
                          const std::string &new_ip_src,
                          const std::string &new_ip_dst,
                          uint16_t new_port_src,
                          uint16_t new_port_dst,
                          db::protocol_type type_of_protocol,
                          const std::string &new_id_sfc,
                          const std::vector<Endpoint> &new_endpoints)
        : item_id(new_item_id), ip_src(new_ip_src), ip_dst(new_ip_dst),
          port_src(new_port_src), port_dst(new_port_dst), prt(type_of_protocol),
          id_sfc(new_id_sfc), endpoints(new_endpoints) {
}

db::DBQuery::Query::Query(const std::string &new_item_id,
                          const std::string &new_ip_src,
                          const std::string &new_ip_dst,
                          uint16_t new_port_src,
                          uint16_t new_port_dst,
                          db::protocol_type type_of_protocol,
                          const std::string &new_id_sfc,
                          const db::endpoint_type &type)
        : item_id(new_item_id), ip_src(new_ip_src), ip_dst(new_ip_dst),
          port_src(new_port_src), port_dst(new_port_dst), prt(type_of_protocol),
          id_sfc(new_id_sfc), endpoints(std::vector<Endpoint>()), type_(type) {}

std::string db::DBQuery::Query::get_ip_src() const {
    return ip_src;
}

std::string db::DBQuery::Query::get_ip_dst() const {
    return ip_dst;
}

uint16_t db::DBQuery::Query::get_port_src() const {
    return port_src;
}

uint16_t db::DBQuery::Query::get_port_dst() const {
    return port_dst;
}

db::protocol_type db::DBQuery::Query::get_protocol_type() const {
    return prt;
}

std::string db::DBQuery::Query::get_id_sfc() const {
    return id_sfc;
}

std::vector<db::DBQuery::Endpoint>
db::DBQuery::Query::get_all_endpoints() const {
    return endpoints;
}

db::DBQuery::Endpoint
db::DBQuery::Query::get_endpoint(db::endpoint_type endpoint) const {
    for (auto e : endpoints) {
        if (e.get_endpoint_typology() == endpoint) {
            return e;
        }
    }
    throw db::exceptions::logic_failure::endpoint_not_found(
            "Endpoint not found");
}

// Pay attention! Here item_id is not included in the JSON file.
std::string db::DBQuery::Query::to_json() const {
    Json::Value json_res;

    json_res[query::endpoint::SRC_IP] = ip_src;
    json_res[query::endpoint::DST_IP] = ip_dst;
    json_res[query::endpoint::SRC_PORT] = port_src;
    json_res[query::endpoint::DST_PORT] = port_dst;
    json_res[query::endpoint::SFC_ID] = id_sfc;
    (prt == protocol_type::TCP) ? json_res[query::endpoint::PROTOCOL] = "tcp" :
            json_res[query::endpoint::PROTOCOL] = "udp";
    for (auto e : endpoints) {
        if (e.get_endpoint_typology() == INGRESS_T) {
            json_res[query::endpoint::SOCK_INGRESS] = e.get_socket_id();
            json_res[query::endpoint::INGRESS_IP] = e.get_ip();
        } else {
            json_res[query::endpoint::SOCK_EGRESS] = e.get_socket_id();
            json_res[query::endpoint::EGRESS_IP] = e.get_ip();
        }
    }

    return sanitize(json_res.toStyledString());
}

std::string db::DBQuery::Query::to_url() const {
    utils::URLBuilder query_builder = utils::URLBuilder()
            .add_path(ENDPOINT_PREFIX);

    if (!endpoints.empty()) {
        if (endpoints.size() == 1) {
            endpoints[0].get_endpoint_typology() == INGRESS_T ?
            query_builder.add_path(query::endpoint::EGRESS) :
            query_builder.add_path(query::endpoint::INGRESS);

            query_builder.add_path(ip_src)
                    .add_path(ip_dst)
                    .add_path(std::to_string(port_src))
                    .add_path(std::to_string(port_dst))
                    .add_path(id_sfc)
                    .add_path(
                            prt == TCP ? "tcp" : "udp"
                    )
                    .add_path(endpoints[0].get_ip())
                    .add_path(endpoints[0].get_socket_id());

            return query_builder.build();
        } else {
            throw exceptions::logic_failure::query_ambiguous("More than one "
                                                             "endpoint for this"
                                                             "query: which should"
                                                             "I use?");
        }
    } else {
        query_builder.add_path(type_ == db::endpoint_type::INGRESS_T ?
                               "ingress" : "egress")
                .add_path(ip_src)
                .add_path(ip_dst)
                .add_path(std::to_string(port_src))
                .add_path(std::to_string(port_dst))
                .add_path(id_sfc)
                .add_path(
                        prt == TCP ? "tcp" : "udp"
                );
        return query_builder.build();
    }
}
// End Query

// Query::Builder
db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_item_id(const std::string &item_id) {
    this->item_id = item_id;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_ip_src(const std::string &ip_src) {
    this->ip_src = ip_src;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_ip_dst(const std::string &ip_dst) {
    this->ip_dst = ip_dst;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_port_src(uint16_t port_src) {
    this->port_src = port_src;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_port_dst(uint16_t port_dst) {
    this->port_dst = port_dst;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_protocol(db::protocol_type prt) {
    this->prt = prt;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_id_sfc(const std::string &id_sfc) {
    this->id_sfc = id_sfc;
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_endpoint(const Endpoint &endpoint) {
    for (auto it = endpoints.begin(); it != endpoints.end(); it++) {
        if (it->get_endpoint_typology() == endpoint.get_endpoint_typology()) {
            endpoints.erase(it);
            endpoints.push_back(endpoint);
            return *this;
        }
    }
    endpoints.push_back(endpoint);
    return *this;
}

db::DBQuery::Query::Builder &
db::DBQuery::Query::Builder::set_endpoint_type(const db::endpoint_type &type) {
    type_ = type;
    return *this;
}

db::DBQuery::Query db::DBQuery::Query::Builder::build() const {
    if (!endpoints.empty())
        return Query(item_id,
                     ip_src,
                     ip_dst,
                     port_src,
                     port_dst,
                     prt,
                     id_sfc,
                     endpoints);
    else
        return Query(item_id,
                     ip_src,
                     ip_dst,
                     port_src,
                     port_dst,
                     prt,
                     id_sfc,
                     type_);
}
// End Query::Builder
