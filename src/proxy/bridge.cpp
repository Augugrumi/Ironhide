//
// Created by zanna on 03/10/18.
//

#include "bridge.h"

proxy::tcp::Bridge::Bridge(boost::asio::io_service &ios)
    : downstream_socket_(ios), upstream_socket_(ios) {}

proxy::tcp::socket_type &proxy::tcp::Bridge::downstream_socket() {
    return downstream_socket_;
}

proxy::tcp::socket_type &proxy::tcp::Bridge::upstream_socket() {
    return upstream_socket_;
}

void proxy::tcp::Bridge::start(const std::string &upstream_host,
                               unsigned short upstream_port) {
    std::cout << "start" << std::endl;
    upstream_socket_.async_connect(
            ip::tcp::endpoint(
                    boost::asio::ip::address::from_string(upstream_host),
                    upstream_port),
            boost::bind(&Bridge::handle_upstream_connect,
                        shared_from_this(),
                        boost::asio::placeholders::error));
}

void proxy::tcp::Bridge::handle_upstream_connect(
        const boost::system::error_code &error) {
    std::cout << "handle_upstream_connect" << std::endl;
    if (!error)
    {
        upstream_socket_.async_read_some(
                boost::asio::buffer(upstream_data_,max_data_length),
                boost::bind(&Bridge::handle_upstream_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));

        downstream_socket_.async_read_some(
                boost::asio::buffer(downstream_data_,max_data_length),
                boost::bind(&Bridge::handle_downstream_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }
    else
        close();
}

void proxy::tcp::Bridge::handle_downstream_write(
        const boost::system::error_code &error) {
    if (!error) {
        upstream_socket_.async_read_some(
                boost::asio::buffer(upstream_data_,max_data_length),
                boost::bind(&Bridge::handle_upstream_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }
    else
        close();
}

void proxy::tcp::Bridge::handle_downstream_read(
        const boost::system::error_code &error,
        const size_t &bytes_transferred) {
    //std::cout << "handle_downstream_read" << std::endl;
    if (!error) {
        async_write(upstream_socket_,
              boost::asio::buffer(downstream_data_,bytes_transferred),
              boost::bind(&Bridge::handle_upstream_write,
                    shared_from_this(),
                    boost::asio::placeholders::error));
    }
    else
        close();
}

void proxy::tcp::Bridge::handle_upstream_write(
        const boost::system::error_code &error) {
    std::cout << "handle_upstream_write" << std::endl;
    if (!error) {
        downstream_socket_.async_read_some(
             boost::asio::buffer(downstream_data_,max_data_length),
             boost::bind(&Bridge::handle_downstream_read,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
    }
    else
        close();
}

void
proxy::tcp::Bridge::handle_upstream_read(const boost::system::error_code &error,
                                         const size_t &bytes_transferred) {
    if (!error) {
        async_write(downstream_socket_,
                    boost::asio::buffer(upstream_data_,bytes_transferred),
                    boost::bind(&Bridge::handle_downstream_write,
                                shared_from_this(),
                                boost::asio::placeholders::error));
    }
    else
        close();
}

void proxy::tcp::Bridge::close() {
    boost::mutex::scoped_lock lock(mutex_);

    if (downstream_socket_.is_open()) {
        downstream_socket_.close();
    }

    if (upstream_socket_.is_open()) {
        upstream_socket_.close();
    }
}

proxy::tcp::Bridge::Acceptor::Acceptor(boost::asio::io_service &io_service,
                                       const std::string &local_host,
                                       unsigned short local_port,
                                       const std::string &upstream_host,
                                       unsigned short upstream_port)
        : io_service_(io_service),
          localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
          acceptor_(io_service_,ip::tcp::endpoint(localhost_address,local_port)),
          upstream_port_(upstream_port),
          upstream_host_(upstream_host) {}

void proxy::tcp::Bridge::Acceptor::handle_accept(
        const boost::system::error_code &error) {
    std::cout << "handle_accept" << std::endl;
    if (!error) {
        session_->start(upstream_host_,upstream_port_);

        if (!accept_connections()) {
            std::cerr << "Failure during call to accept." << std::endl;
        }
    }
    else {
        std::cerr << "Error: " << error.message() << std::endl;
    }
}

bool proxy::tcp::Bridge::Acceptor::accept_connections() {
    std::cout << "accept_connections" << std::endl;
    try {
        session_ = boost::shared_ptr<Bridge>(new Bridge(io_service_));

        acceptor_.async_accept(session_->downstream_socket(),
                               boost::bind(&Acceptor::handle_accept,
                                           this,
                                           boost::asio::placeholders::error));
    }
    catch(std::exception& e) {
        std::cerr << "acceptor exception: " << e.what() << std::endl;
        return false;
    }

    return true;
}

