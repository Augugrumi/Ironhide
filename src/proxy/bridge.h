//
// Created by zanna on 03/10/18.
//

#ifndef IRONHIDE_TCPPROXYSERVER_H
#define IRONHIDE_TCPPROXYSERVER_H


#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>


namespace proxy {
namespace tcp {

class Bridge;

namespace ip = boost::asio::ip;
typedef ip::tcp::socket socket_type;
typedef boost::shared_ptr<Bridge> ptr_type;

class Bridge : public boost::enable_shared_from_this<Bridge> {
    private:
        void handle_downstream_write(const boost::system::error_code& error);
        void handle_downstream_read(const boost::system::error_code& error,
                                    const size_t& bytes_transferred);
        void handle_upstream_write(const boost::system::error_code& error);
        void handle_upstream_read(const boost::system::error_code& error,
                                  const size_t& bytes_transferred);
        void close();

        socket_type downstream_socket_;
        socket_type upstream_socket_;
        enum { max_data_length = 8192 }; //8KB
        unsigned char downstream_data_[max_data_length];
        unsigned char upstream_data_[max_data_length];
        boost::mutex mutex_;

    public:

        Bridge(boost::asio::io_service& ios);
        socket_type& downstream_socket();
        socket_type& upstream_socket();
        void start(const std::string& upstream_host, unsigned short upstream_port);
        void handle_upstream_connect(const boost::system::error_code& error);

        class Acceptor {
            private:
                boost::asio::io_service& io_service_;
                ip::address_v4 localhost_address;
                ip::tcp::acceptor acceptor_;
                ptr_type session_;
                unsigned short upstream_port_;
                std::string upstream_host_;

                void handle_accept(const boost::system::error_code& error);
            public:
                Acceptor(boost::asio::io_service& io_service,
                const std::string& local_host, unsigned short local_port,
                const std::string& upstream_host, unsigned short upstream_port);
                bool accept_connections();
        };

};

} // namespace tcp
} // namespace proxy
#endif //IRONHIDE_TCPPROXYSERVER_H
