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

void db::DBQuery::create_entry(char *ip_src, char *ip_dst,
                               uint16_t port_src, uint16_t port_dst,
                               db::protocol_type protocol,
                               char *id_sfc,
                               db::endpoint_type endpoint,
                               char* endpoint_ip, int endpoint_socket) {
    std::string req_addr;
    req_addr.append(roulette_addr.get_URL())
            .append("endpoints/");

    curl_easy_setopt(curl, CURLOPT_URL, req_addr.c_str());

    Json::Value val;
    val[SRC_IP] = ip_src;
    val[DST_IP] = ip_dst;
    val[SRC_PORT] = port_src;
    val[DST_PORT] = port_dst;
    val[SFC_ID] = id_sfc;
    (protocol == protocol_type::TCP)? val[PROTOCOL] = "tcp" :
                                      val[PROTOCOL] = "udp";

    std::string req_data_res;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req_data_res);

    const char* sock;
    const char* ip;
    (endpoint == INGRESS_T)? sock = SOCK_INGRESS, ip = INGRESS_IP :
                             sock = SOCK_EGRESS, ip = EGRESS_IP;
    val[sock] = endpoint_socket;
    val[ip] = endpoint_ip;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, val.asCString());


    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
}

void db::DBQuery::update_endpoint(char* ip_src, char* ip_dst,
                                  uint16_t port_src, uint16_t port_dst,
                                  protocol_type protocol,
                                  char* id_sfc,
                                  db::endpoint_type endpoint,
                                  char *endpoint_ip, int endpoint_socket) {
    // TODO make the request before the update. What is request?
    std::string req_addr;
    req_addr.append(roulette_addr.get_URL())
            .append("endpoints/");
    (endpoint == INGRESS_T)? req_addr.append(INGRESS) :
                             req_addr.append(EGRESS);
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
        val[INGRESS_IP] = endpoint_ip;
        val[SOCK_INGRESS] = endpoint_socket;
    } else {
        val[EGRESS_IP] = endpoint_ip;
        val[SOCK_EGRESS] = endpoint_socket;
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

