#include "egress.h"

endpoint::Egress::Egress(uint16_t ext_port, uint16_t int_port) :
        Endpoint(ext_port, int_port) {}


void endpoint::Egress::manage_exiting_udp_packets(unsigned char *pkt,
                                                  size_t pkt_len,
                                                  const ConnectionEntry &ce,
                                                  socket_fd sock) {
    const unsigned int pkt_calc = SFC_HDR + IP_UDP_H_LEN(pkt + SFC_HDR);

    LOG(ldebug, "manage_exiting udp packets");
    auto header = utils::sfc_header::SFCUtilities::retrieve_header(pkt);

    LOG(ldebug, "destination: addr: " + INT_TO_IP(header.destination_address));
    LOG(ldebug,
        "destination: port: " + std::to_string(htons(header.destination_port)));

    // raw socket to send data
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create raw socket");
        exit(EXIT_FAILURE);
    }

    unsigned char datagram[BUFFER_SIZE];
    unsigned char *total_pkt = datagram;
    struct iphdr *iph;
    struct udphdr *udph;
    utils::PacketUtils::forge_ip_udp_pkt(pkt + pkt_calc, pkt_len - pkt_calc,
                                         get_my_ip().c_str(),
                                         ce.get_ip_dst().c_str(),
                                         get_external_port(), ce.get_port_dst(),
                                         iph, udph,
                                         total_pkt);


    int raw_socket;
    struct sockaddr_in sockstr{};
    socklen_t socklen;

    /* no pointer to array!
     * >> It was like "a variable that contains an address -- and in this
     *    address begins an array of chars"! */
    /* now it is simple an array of chars :-)  */
    char msg[BUFFER_SIZE];

    /* do not use IPPROTO_RAW to receive packets */
    if ((raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE); /* here there is no clean up -- retval was not used */
    }

    sockstr.sin_family = AF_INET;
    sockstr.sin_port = htons(get_external_port());//ce.get_port_dst());
    sockstr.sin_addr.s_addr = inet_addr(
            get_my_ip().c_str());//inet_addr(ce.get_ip_dst().c_str());
    socklen = (socklen_t) sizeof(sockstr);

    if (bind(raw_socket, (struct sockaddr *) &sockstr, socklen) == -1) {
        LOG(lfatal, "raw socket bind");
        close(raw_socket);
        exit(EXIT_FAILURE);
    }

    memset(msg, 0, BUFFER_SIZE);

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(header.destination_port);
    server.sin_addr.s_addr = header.destination_address;

    if (sendto(sock, datagram, iph->tot_len, 0,
               (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    } else {
        LOG(ldebug, "Packet Send ");
    }

    update_entry(ce, sock, server, db::endpoint_type::EGRESS_T);

    ssize_t received_len;
    auto buffer = new unsigned char[BUFFER_SIZE];

    char *sfcid;
    uint16_t next_port;
    unsigned long ttl;

    do {
        received_len = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *) &sockstr,
                                &(socklen));
        if (received_len < 0) {
            LOG(lfatal, "error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            sfcid = classifier_.classify_pkt(buffer,
                                             static_cast<size_t>(received_len));
            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(
                            static_cast<uint32_t>(std::stoi(sfcid)));

            if (!path.empty()) {
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                next_port = static_cast<uint16_t>(path[0].get_port());

                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                static_cast<uint32_t>(std::stoi(sfcid)), 0,
                                INT_TO_IP_C_STR(header.source_address),
                                header.source_port,
                                INT_TO_IP_C_STR(header.destination_address),
                                header.destination_port,
                                static_cast<uint16_t>(ttl), 1);

                std::vector<unsigned char> formatted_pkt(
                        static_cast<unsigned long>(received_len + SFC_HDR));
                unsigned char *pkt_pointer = &formatted_pkt[0];

                utils::sfc_header::SFCUtilities::prepend_header(buffer,
                                                                static_cast<size_t>(received_len),
                                                                flh,
                                                                pkt_pointer);

                client::udp::ClientUDP().send_and_wait_response(pkt_pointer,
                                                                static_cast<size_t>(
                                                                        received_len +
                                                                        SFC_HDR),
                                                                path[0].get_address().c_str(),
                                                                next_port);

            } else {
                LOG(ldebug, "no route available, discarding packages");
            }
            if (received_len < BUFFER_SIZE)
                break;
        }
    } while (received_len > 0);

    free(pkt);
    free(buffer);

    LOG(ldebug, "exit managing UDP exiting pkts");

    // TODO think when closing socket
}


void endpoint::Egress::manage_exiting_tcp_packets(unsigned char *pkt,
                                                  size_t pkt_len,
                                                  const ConnectionEntry &ce,
                                                  socket_fd socket) {
    LOG(ldebug, "manage_exiting tcp packets");
    auto header = utils::sfc_header::SFCUtilities::retrieve_header(pkt);
    const unsigned int pkt_calc = SFC_HDR + IP_TCP_H_LEN(pkt + SFC_HDR);

    std::vector<unsigned char> v(pkt_len - pkt_calc);
    unsigned char *s = &v[0];
    std::strncpy((char *) s, (char *) (pkt + pkt_calc), pkt_len - pkt_calc);

    if (socket == -1) {
        client::tcp::ClientTCP client{};
        LOG(ldebug, utils::PacketUtils::int_to_ip(header.destination_address));
        LOG(ldebug, std::to_string(htons(header.destination_port)));
        client.connect_to_server(INT_TO_IP_C_STR(header.destination_address),
                                 htons(header.destination_port));
        socket = client.access_to_socket();
        sockaddr_in sockin = client.access_to_sockaddr_in();
        update_entry(ce, socket, sockin, db::endpoint_type::EGRESS_T);
    }

    LOG(ldebug, std::to_string(pkt_len));

    fcntl(socket, F_GETFL, 0);

    unsigned char received[BUFFER_SIZE];

    ssize_t received_len = 0;
    send(socket, s, pkt_len - pkt_calc, 0);
    char *sfcid;
    uint16_t next_port;
    unsigned long ttl;
    do {
        received_len = read(socket, received, BUFFER_SIZE);
        if (received_len < 0) {
            LOG(lfatal, "error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            sfcid = classifier_.classify_pkt(received,
                                             static_cast<size_t>(received_len));
            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(
                            static_cast<uint32_t>(std::stoi(sfcid)));

            for (const db::utils::Address &a : path) {
                LOG(ldebug, "address:" + a.get_address());
                LOG(ldebug, "url:" + a.get_URL());
            }

            if (!path.empty()) {
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                db::utils::Address a = path.at(0);
                next_port = static_cast<uint16_t>(path[0].get_port());
                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                static_cast<uint32_t>(std::stoi(sfcid)), 0,
                                INT_TO_IP_C_STR(header.source_address),
                                header.source_port,
                                INT_TO_IP_C_STR(header.destination_address),
                                header.destination_port,
                                static_cast<uint16_t>(ttl), 1);
                std::vector<unsigned char> formatted_pkt(
                        static_cast<unsigned long>(received_len + SFC_HDR));
                unsigned char *pkt_pointer = &formatted_pkt[0];
                utils::sfc_header::SFCUtilities::prepend_header(received,
                                                                static_cast<size_t>(received_len),
                                                                flh,
                                                                pkt_pointer);
                LOG(ldebug, "next ip   " + std::string(a.get_address()));
                LOG(ldebug, "next port " + std::to_string(next_port));
                client::udp::ClientUDP().send_and_wait_response(pkt_pointer,
                                                                static_cast<size_t>(
                                                                        received_len +
                                                                        SFC_HDR),
                                                                a.get_address().c_str(),
                                                                next_port);


            } else {
                LOG(lwarn, "no route available, discarding packages");
                break;
            }

            if (received_len < BUFFER_SIZE) {
                LOG(ltrace, "break");
                break;
            }
        } else {
            break;
        }
    } while (true);

    LOG(ldebug, "exit managing TCP exiting pkts");

    // TODO think when closing socket
}

void endpoint::Egress::manage_pkt_from_chain(void *mngmnt_args) {
    LOG(ldebug, "manage_pkt_from_chain");
    auto args = (server::udp::udp_pkt_mngmnt_args *) mngmnt_args;

    sendto(args->socket_fd,
           ACK,
           ACK_SIZE,
           0,
           reinterpret_cast<struct sockaddr *>(&args->client_address),
           sizeof(args->client_address));

    sfc_header flh = utils::sfc_header::SFCUtilities::retrieve_header(
            (unsigned char *) args->pkt);

    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char *) (args->pkt + SFC_HDR));

    // TODO think something more extensible to support more protocols
    // '6' is the code for TCP '17' for UDP
    db::protocol_type conn_type = (headers.first.protocol == 6 ?
                                   db::protocol_type::TCP :
                                   db::protocol_type::UDP);

    ConnectionEntry ce(utils::PacketUtils::int_to_ip(flh.source_address),
                       utils::PacketUtils::int_to_ip(flh.destination_address),
                       htons(flh.source_port), htons(flh.destination_port),
                       std::to_string(flh.p_id), conn_type);

    args->socket_fd = retrieve_connection_2(ce).first;

    LOG(ldebug, "pkt_len: " + std::to_string(args->pkt_len));

    if (conn_type == db::protocol_type::TCP) {
        std::function<void()> f = [this, args, ce]() {

            manage_exiting_tcp_packets((unsigned char *) args->pkt,
                                       static_cast<size_t>(args->pkt_len),
                                       ce, args->socket_fd);
        };

        GO_ASYNC(f);
    } else {
        std::function<void()> f = [this, args, ce]() {
            manage_exiting_udp_packets((unsigned char *) args->pkt,
                                       static_cast<size_t>(args->pkt_len),
                                       ce, args->socket_fd);
        };

        GO_ASYNC(f);
    };

}

void endpoint::Egress::start() {
    uint16_t int_port = get_internal_port();
    std::function<void(void *)> vnf_callback = [this](void *args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_internal_thread([&int_port, &vnf_callback] {
        LOG(ldebug, "Starting server udp for internal pkts on port "
                    + std::to_string(int_port));
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    udp_internal_thread.join();
}
