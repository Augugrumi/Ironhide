//
// Created by zanna on 05/10/18.
//

#include "dbquery.h"

db::DBQuery::DBQuery(const db::Address& r_a) : roulette_addr(r_a) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl == nullptr) {
        perror("Impossible to properly init curl");
        exit(EXIT_FAILURE);
    }
}

db::DBQuery::DBQuery(const std::string& address, uint16_t port)
        : db::DBQuery(db::Address(address, port)) {}

size_t db::DBQuery::curl_callback(void* ptr, size_t size,
                                  size_t nmemb, std::string* data) {
    data->append(reinterpret_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

bool db::DBQuery::create_entry(char *ip_src, char *ip_dst,
                               uint16_t port_src, uint16_t port_dst,
                               db::protocol_type protocol,
                               char *id_sfc,
                               db::endpoint_type endpoint,
                               char* endpoint_ip, int endpoint_socket) {
    std::string req_addr;
    req_addr.append(roulette_addr.get_URL())
            .append(ENDPOINT_PREFIX);

    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());

    Json::Value val;
    val[query::SRC_IP] = ip_src;
    val[query::DST_IP] = ip_dst;
    val[query::SRC_PORT] = port_src;
    val[query::DST_PORT] = port_dst;
    val[query::SFC_ID] = id_sfc;
    (protocol == protocol_type::TCP)? val[query::PROTOCOL] = "tcp" :
                                      val[query::PROTOCOL] = "udp";

    std::string req_data_res;


    const char* sock;
    const char* ip;

    if (endpoint == INGRESS_T) {
        sock = query::SOCK_INGRESS;
        ip = query::INGRESS_IP;
    } else {
        sock = query::SOCK_EGRESS;
        ip = query::EGRESS_IP;
    }
    val[sock] = endpoint_socket;
    val[ip] = endpoint_ip;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, val.asCString());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, db::DBQuery::curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    } else {
        BOOST_LOG_TRIVIAL(debug) << "Request performed successfully";

        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        Json::Value response;

        std::string errors;
        bool read = reader->parse(req_data_res.c_str(),
                                  req_data_res.c_str() + req_data_res.size(),
                                  &response,
                                  &errors);

        if (read) { // Read perform successfully
            if (response[reply::REASON] == reply::OK) { // The query is ok
                return true;
            } else {
                return false;
            }
        } else {
            throw exceptions::ios_base::failure("Error while making the request"
                                                "to route backend");
        }
    }
}

db::Address db::DBQuery::update_endpoint(char* ip_src, char* ip_dst,
                                  uint16_t port_src, uint16_t port_dst,
                                  protocol_type protocol,
                                  char* id_sfc,
                                  db::endpoint_type endpoint,
                                  char *endpoint_ip, int endpoint_socket) {
    // TODO make the request before the update. What is request?
    std::string req_addr;
    req_addr.append(roulette_addr.get_URL())
            .append(ENDPOINT_PREFIX);
    (endpoint == INGRESS_T)? req_addr.append(query::INGRESS) :
                             req_addr.append(query::EGRESS);
    req_addr.append("/")
            .append(ip_src)
            .append(ip_dst)
            .append(std::to_string(port_src))
            .append(std::to_string(port_dst))
            .append(id_sfc);
    (protocol == TCP)? req_addr.append("tcp") :
                       req_addr.append("udp");

    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */
    std::string req_data_res;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

    Json::Value val;
    if(endpoint == INGRESS_T) {
        val[query::INGRESS_IP] = endpoint_ip;
        val[query::SOCK_INGRESS] = endpoint_socket;
    } else {
        val[query::EGRESS_IP] = endpoint_ip;
        val[query::SOCK_EGRESS] = endpoint_socket;
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, val.asCString());

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    curl_easy_cleanup(curl);
}

db::DBQuery::~DBQuery() {
    if (curl != nullptr) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

