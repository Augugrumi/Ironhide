//
// Created by zanna on 05/10/18.
//
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <iterator>
#include <ostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include "egress.h"

endpoint::Egress::Egress(uint16_t ext_port, uint16_t int_port) :
        Endpoint(ext_port, int_port){}





void endpoint::Egress::manage_exiting_udp_packets(unsigned char* pkt,
                                                  size_t pkt_len,
                                                  const ConnectionEntry& ce,
                                                  socket_fd sock) {
    const unsigned int pkt_calc = SFC_HDR + IP_UDP_H_LEN(pkt + SFC_HDR);

    LOG(ldebug, "manage_exiting udp packets");
    auto header = utils::sfc_header::SFCUtilities::retrieve_header(pkt);
    //client::udp::ClientUDP client;


    printf("message to send\n");
    for(int i = 0; i < pkt_len; i++)
        printf("%x", *(pkt + i));
    printf("\n");

    LOG(ldebug, "destination: addr: " + INT_TO_IP(header.destination_address));
    LOG(ldebug, "destination: port: " + std::to_string(htons(header.destination_port)));

    // raw socket to send data
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create raw socket");
        exit(EXIT_FAILURE);
    }

    unsigned char datagram[BUFFER_SIZE];
    unsigned char* total_pkt = datagram;
    struct iphdr* iph;
    struct udphdr* udph;
    utils::PacketUtils::forge_ip_udp_pkt(pkt + pkt_calc, pkt_len - pkt_calc,
            get_my_ip().c_str(), ce.get_ip_dst().c_str(),
            get_external_port(), ce.get_port_dst(),
            iph, udph,
            total_pkt);


    int raw_socket;
    struct sockaddr_in sockstr;
    socklen_t socklen;

    int retval = 0; /* the return value (give a look when an error happens)
                     */

    /* no pointer to array!
     * >> It was like "a variable that contains an address -- and in this
     *    address begins an array of chars"! */
    /* now it is simple an array of chars :-)  */
    char msg[BUFFER_SIZE];
    ssize_t msglen; /* return value from recv() */

    /* do not use IPPROTO_RAW to receive packets */
    if ((raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE); /* here there is no clean up -- retval was not used */
    }

    sockstr.sin_family = AF_INET;
    sockstr.sin_port = htons(ce.get_port_dst());
    sockstr.sin_addr.s_addr = inet_addr(get_my_ip().c_str());//inet_addr(ce.get_ip_dst().c_str());
    socklen = (socklen_t) sizeof(sockstr);

    if (bind(raw_socket, (struct sockaddr*) &sockstr, socklen) == -1) {
        perror("raw socket bind");
        close(raw_socket);
        exit(EXIT_FAILURE);
    }

    memset(msg, 0, BUFFER_SIZE);

    struct sockaddr_in server;
    server.sin_family      = AF_INET;
    server.sin_port        = htons(header.destination_port);
    server.sin_addr.s_addr = header.destination_address;

    if (sendto(sock, datagram, iph->tot_len , 0,
               (struct sockaddr *) &server, sizeof (server)) < 0) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    } else {
        LOG(ldebug, "Packet Send");
    }


    /*socket = client.send_only(pkt + pkt_calc, // move pointer after headers
                              pkt_len - pkt_calc, // resize without considering headers
                              INT_TO_IP_C_STR(header.destination_address),
                              htons(header.destination_port));*/

    update_entry(ce, sock, server, db::endpoint_type::EGRESS_T);

    ssize_t received_len;
    auto buffer = new unsigned char[BUFFER_SIZE];

    socklen_t server_addr_len = sizeof(server);
    char* sfcid;
    char* next_ip;
    uint16_t next_port;
    unsigned long ttl;

    do {
        received_len = recv(raw_socket,buffer, BUFFER_SIZE,0);/*,0,
                                (struct sockaddr *)&server,
                                &(server_addr_len));*/
        printf("Message size: %d\n", received_len + SFC_HDR);

        if (received_len < 0) {
            perror("error receiving data");
            //exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            sfcid = classifier_.classify_pkt(buffer, received_len);
            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(atoi(sfcid));

            if (!path.empty()) {
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                next_ip = const_cast<char*>(path[0].get_address().c_str());
                next_port = path[0].get_port();

                auto resp_h = utils::PacketUtils::retrieve_ip_udp_header(buffer);

                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                atoi(sfcid), 0,
                                INT_TO_IP_C_STR(header.source_address),
                                header.source_port,
                                INT_TO_IP_C_STR(header.destination_address),
                                header.destination_port, ttl, 1);

                unsigned char formatted_pkt[received_len + SFC_HDR];
                unsigned char* pkt_pointer = formatted_pkt;
                utils::sfc_header::SFCUtilities::prepend_header(buffer,
                                                                received_len,
                                                                flh,
                                                                pkt_pointer);

                client::udp::ClientUDP().send_and_wait_response(pkt_pointer,
                                                                received_len + SFC_HDR,
                                                                next_ip,
                                                                next_port);

            } else {
                LOG(ldebug, "no route available, discarding packages");
            }
            if (received_len < BUFFER_SIZE)
                break;
        }
    } while(received_len > 0);

    free(pkt);
    free(buffer);

    LOG(ldebug, "exit managing UDP exiting pkts");

    // TODO think when closing socket
}


void endpoint::Egress::manage_exiting_tcp_packets(unsigned char* pkt,
                                                  size_t pkt_len,
                                                  const ConnectionEntry& ce,
                                                  socket_fd socket) {
    LOG(ldebug, "manage_exiting tcp packets");
    auto header = utils::sfc_header::SFCUtilities::retrieve_header(pkt);
    auto headers = utils::PacketUtils::retrieve_ip_tcp_header(pkt + SFC_HDR);
    const unsigned int pkt_calc = SFC_HDR + IP_TCP_H_LEN(pkt + SFC_HDR);

    printf("message to send\n");
    for(int i = pkt_calc; i < pkt_len; i++)
        printf("%x", *(pkt + i));
    printf("\n");

    for(int i = pkt_calc; i < pkt_len; i++)
        printf("%x", *(pkt + i));
    printf("\n");

    unsigned char s[pkt_len - pkt_calc];
    std::strncpy((char*)s, (char*)(pkt + pkt_calc), pkt_len - pkt_calc);
    for(int i = 0; i < pkt_len - pkt_calc; i++)
        printf("%x", *(s + i));
    printf("\n");

    printf("%s\n", INT_TO_IP(headers.first.daddr).c_str());
    printf("%d\n", htons(headers.second.dest));

    LOG(ldebug, "1");
    if (socket == -1) {
        client::tcp::ClientTCP client;
        LOG(ldebug, "a");
        LOG(ldebug, utils::PacketUtils::int_to_ip(header.destination_address));
        LOG(ldebug, std::to_string(htons(header.destination_port)));
        client.connect_to_server(INT_TO_IP_C_STR(header.destination_address),
                                 htons(header.destination_port));
        LOG(ldebug, "b");
        socket = client.access_to_socket();
        sockaddr_in sockin = client.access_to_sockaddr_in();
        LOG(ldebug, "c");
        update_entry(ce, socket, sockin, db::endpoint_type::EGRESS_T);
        LOG(ldebug, "d");
    }

    printf("message to send\n");
    for(int i = 0; i < pkt_len; i++)
        printf("%x", *(pkt + i));
    printf("\n");
    printf("%s\n", INT_TO_IP(headers.first.daddr).c_str());
    printf("%d\n", htons(headers.second.dest));

    LOG(ldebug, "2");
    LOG(ldebug, std::to_string(pkt_len));
    //size_t iTotalElement = *(&pkt + 1) - pkt;

    fcntl(socket, F_GETFL, 0);

    //unsigned char* c;
    //client.send_and_receive(s, pkt_len - pkt_calc + 1, c, 0);

    unsigned char received[BUFFER_SIZE];

    for (int i = 0; i < pkt_len - pkt_calc; i++)
        printf("%x", *(s + i));
    printf("\n");

    ssize_t received_len = 0;
    send(socket, s, pkt_len - pkt_calc, 0);
    char* sfcid;
    char* next_ip;
    uint16_t next_port;
    unsigned long ttl;
    do {
        printf("Hello message sent\n");
        received_len = read(socket , received, BUFFER_SIZE);
        for (int i = 0; i < received_len; i++)
            printf("%x", *(received + i));
        printf("\n");
        if (received_len < 0) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            LOG(ldebug, "6");
            sfcid = classifier_.classify_pkt(received, received_len);
            LOG(ldebug, "7");
            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(atoi(sfcid));
            LOG(ldebug, "8");

            for (db::utils::Address a : path) {
                LOG(ldebug, "address:" + a.get_address());
                LOG(ldebug, "url:" + a.get_URL());
            }

            if (!path.empty()) {
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                db::utils::Address a = path.at(0);
                next_port = path[0].get_port();
                LOG(ldebug, "9");
                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                atoi(sfcid), 0,
                                INT_TO_IP_C_STR(header.source_address),
                                header.source_port,
                                INT_TO_IP_C_STR(header.destination_address),
                                header.destination_port, ttl, 1);
                unsigned char formatted_pkt[received_len + SFC_HDR];
                unsigned char* pkt_pointer = formatted_pkt;
                LOG(ldebug, "10");
                utils::sfc_header::SFCUtilities::prepend_header(received,
                                                                received_len,
                                                                flh,
                                                                pkt_pointer);
                LOG(ldebug, "11");
                LOG(ldebug, "next ip   " + std::string(a.get_address()));
                LOG(ldebug, "next port " + std::to_string(next_port));
                client::udp::ClientUDP().send_and_wait_response(pkt_pointer,
                                                                received_len + SFC_HDR,
                                                                a.get_address().c_str(),
                                                                next_port);



            } else {
                LOG(ldebug, "no route available, discarding packages");
                break;
            }

            if (received_len < BUFFER_SIZE) {
                break;
            }
        }/* else {
            break;
        }*/
    } while(1);

    /*do {
        printf("entering do while");
        received_len = read(socket , received, BUFFER_SIZE);

        for (int i = 0; i < received_len; i++)
            printf("%x", *(received + i));
        printf("\n");

        if (received_len < BUFFER_SIZE) {
            break;
        }

    } while(received_len > 0);*/

    //exit(EXIT_FAILURE);

    /*send(socket, s, pkt_len - pkt_calc, 0);

    LOG(ldebug, "3");
    //ssize_t received_len = 0;
    unsigned char buffer[BUFFER_SIZE];
    //char* sfcid;
    //char* next_ip;
    //uint16_t next_port;
    //unsigned long ttl;

    do {
        LOG(ldebug, "4");
        received_len = read(socket, buffer, BUFFER_SIZE);
        printf("received ");
        for(int i = 0; i < received_len; i++)
            printf("%x", buffer);
        printf("\n");
        LOG(ldebug, "5");
        if (received_len < 0) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            LOG(ldebug, "6");
            sfcid = classifier_.classify_pkt(buffer, received_len);
            LOG(ldebug, "7");
            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(atoi(sfcid));
            LOG(ldebug, "8");

            if (!path.empty()) {
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                next_ip = const_cast<char *>(path[0].get_address().c_str());
                next_port = path[0].get_port();
                LOG(ldebug, "9");
                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                atoi(sfcid), 0,
                                INT_TO_IP_C_STR(header.source_address),
                                header.source_port,
                                INT_TO_IP_C_STR(header.destination_address),
                                header.destination_port, ttl, 1);
                unsigned char formatted_pkt[pkt_len + SFC_HDR];
                unsigned char* pkt_pointer = formatted_pkt;
                LOG(ldebug, "10");
                utils::sfc_header::SFCUtilities::prepend_header(buffer,
                                                                pkt_len,
                                                                flh,
                                                                pkt_pointer);
                LOG(ldebug, "11");
                LOG(ldebug, "next ip   " + std::string(next_ip));
                LOG(ldebug, "next port " + std::to_string(next_port));
                client::udp::ClientUDP().send_and_wait_response(pkt_pointer,
                                                                pkt_len + SFC_HDR,
                                                                next_ip,
                                                                next_port);



            } else {
                LOG(ldebug, "no route available, discarding packages");
            }
        }

        if (received_len < BUFFER_SIZE) {
            break;
        }

    }  while(1);*/

    //free(pkt);
    //free(buffer);

    LOG(ldebug, "exit managing TCP exiting pkts");

    // TODO think when closing socket
}

#include <stdio.h>
void endpoint::Egress::manage_pkt_from_chain(void * mngmnt_args) {
    LOG(ldebug, "manage_pkt_from_chain");
    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    sendto(args->socket_fd,
           ACK,
           ACK_SIZE,
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));

    sfc_header flh = utils::sfc_header::SFCUtilities::retrieve_header(
            (unsigned char*)args->pkt);

    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)(args->pkt + SFC_HDR));

    // TODO think something more extensible to support more protocols
    // '6' is the code for TCP '17' for UDP
    db::protocol_type conn_type = (headers.first.protocol == 6?
                                        db::protocol_type::TCP :
                                        db::protocol_type::UDP);

    ConnectionEntry ce(utils::PacketUtils::int_to_ip(flh.source_address),
                       utils::PacketUtils::int_to_ip(flh.destination_address),
                       htons(flh.source_port), htons(flh.destination_port),
                       std::to_string(flh.p_id), conn_type);

    args->socket_fd = retrieve_connection_2(ce).first;

    LOG(ldebug, "pkt_len: " + std::to_string(args->pkt_len));

    if (conn_type == db::protocol_type::TCP) {
        std::function<void ()> f = [this, args, ce]() {

            manage_exiting_tcp_packets((unsigned char*)args->pkt,
                                       args->pkt_len,
                                       ce, args->socket_fd);
        };

        GO_ASYNC(f);
    } else {
        std::function<void ()> f = [this, args, ce]() {
            manage_exiting_udp_packets((unsigned char*)args->pkt, args->pkt_len,
                                       ce, args->socket_fd);
        };

        GO_ASYNC(f);
    };

}

void endpoint::Egress::start() {
    uint16_t int_port = get_internal_port();
    std::function<void (void*)> vnf_callback = [this](void* args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_internal_thread([&int_port, &vnf_callback]{
        LOG(ldebug, "Starting server udp for internal pkts on port "
                    + std::to_string(int_port));
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    udp_internal_thread.join();
}