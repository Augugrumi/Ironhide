//
// Created by zanna on 05/10/18.
//

#include "dbquery.h"

db::DBQuery::DBQuery(const utils::Address& r_a) : roulette_addr(r_a) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl == nullptr) {
        perror("Impossible to properly init curl");
        exit(EXIT_FAILURE);
    }
}

db::DBQuery::DBQuery(const std::string& address, uint16_t port)
        : db::DBQuery(utils::Address(address, port)) {}

size_t db::DBQuery::curl_callback(void* ptr, size_t size,
                                  size_t nmemb, std::string* data) {
    data->append(reinterpret_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::string db::DBQuery::create_entry(const char *ip_src, const char *ip_dst,
                               uint16_t port_src, uint16_t port_dst,
                               protocol_type protocol,
                               const char *id_sfc,
                               endpoint_type endpoint,
                               const char* endpoint_ip,
                               const char* endpoint_socket) {
    struct curl_slist* header = nullptr;
    const char* sock;
    const char* ip;
    Json::Value val;
    std::string req_data_res;
    std::string req_addr = utils::URLBuilder()
            .set_address(roulette_addr)
            .add_path(ENDPOINT_PREFIX)
            .build();

    val[query::SRC_IP] = ip_src;
    val[query::DST_IP] = ip_dst;
    val[query::SRC_PORT] = port_src;
    val[query::DST_PORT] = port_dst;
    val[query::SFC_ID] = id_sfc;
    (protocol == protocol_type::TCP)? val[query::PROTOCOL] = "tcp" :
                                      val[query::PROTOCOL] = "udp";

    if (endpoint == INGRESS_T) {
        sock = query::SOCK_INGRESS;
        ip = query::INGRESS_IP;
    } else {
        sock = query::SOCK_EGRESS;
        ip = query::EGRESS_IP;
    }
    val[sock] = endpoint_socket;
    val[ip] = endpoint_ip;

    std::string json = val.toStyledString();
    std::replace(json.begin(), json.end(), '\0', ' ');

    LOG(ldebug, "URL to send data: " + req_addr);
    LOG(ldebug, "Data to send: " + json);

    header = curl_slist_append(header, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, db::DBQuery::curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(header);
    if (handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res))) {

    }


    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value response;

    std::string errors;
    LOG(ldebug, "Data obtained: " + req_data_res);
    reader->parse(req_data_res.c_str(),
                  req_data_res.c_str() + req_data_res.size(),
                  &response,
                  &errors);


    return response[reply::CONTENT]["id"].toStyledString();
}

bool db::DBQuery::update_endpoint(const char* ip_src, const char* ip_dst,
                                  uint16_t port_src, uint16_t port_dst,
                                  protocol_type protocol,
                                  const char* id_sfc,
                                  endpoint_type endpoint,
                                  const char* actual_ip,
                                  const char* actual_socket,
                                  const char* new_ip,
                                  const char* new_socket_id) {

    struct curl_slist* header = nullptr;
    Json::Value val;
    std::string req_data_res;
    std::string kind_of_id;
    std::string kind_of_ip;
    std::string type_of_xgress;
    std::string type_of_protocol;
    std::string req_addr;

    if (endpoint == INGRESS_T) {
        req_addr.append(query::INGRESS);
        kind_of_id = query::SOCK_EGRESS;
        kind_of_ip = query::EGRESS_IP;
        type_of_xgress = query::EGRESS;
    } else {
        req_addr.append(query::EGRESS);
        kind_of_id = query::SOCK_INGRESS;
        kind_of_ip = query::INGRESS_IP;
        type_of_xgress = query::INGRESS;
    }
    (protocol == TCP)? type_of_protocol.append("tcp") :
                       type_of_protocol.append("udp");

    req_addr = utils::URLBuilder()
            .set_address(roulette_addr)
            .add_path(ENDPOINT_PREFIX)
            .add_path(type_of_xgress)
            .add_path(ip_src)
            .add_path(ip_dst)
            .add_path(std::to_string(port_src))
            .add_path(std::to_string(port_dst))
            .add_path(id_sfc)
            .add_path(type_of_protocol)
            .add_path(actual_ip)
            .add_path(actual_socket)
            .build();

    val[kind_of_ip] = new_ip;
    val[kind_of_id] = new_socket_id;

    std::string json = val.toStyledString();
    std::replace(json.begin(), json.end(), '\0', ' ');

    header = curl_slist_append(header, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, db::DBQuery::curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

    CURLcode res = curl_easy_perform(curl);
    return handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res));
}

bool db::DBQuery::delete_endpoint(const char* id) {
    const std::string req_addr = utils::URLBuilder()
            .set_address(roulette_addr)
            .add_path(ENDPOINT_PREFIX)
            .add_path(id)
            .build();
    std::string req_data_res;

    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, db::DBQuery::curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

    CURLcode res = curl_easy_perform(curl);
    return handle_req(
                res,
                std::bind<bool>(&db::DBQuery::is_op_ok, this, req_data_res));
}

bool db::DBQuery::handle_req(const CURLcode& res, std::function<bool()> cb) {
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        throw exceptions::ios_base::failure("Error while making the request"
                                            "to route backend");
    } else {
        return cb();
    }
}

bool db::DBQuery::is_op_ok(const std::string& req) {
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
        throw exceptions::ios_base::failure("Error while parsing the JSON"
                                            " reply");
    }
}

db::DBQuery::~DBQuery() {
    if (curl != nullptr) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

