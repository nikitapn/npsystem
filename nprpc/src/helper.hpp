#pragma once

#include <boost/asio.hpp>
#include <nprpc/endpoint.hpp>


namespace nprpc::impl {

boost::asio::ip::tcp::endpoint
sync_socket_connect(const EndPoint& endpoint, boost::asio::ip::tcp::socket& socket);

}  // namespace nprpc::impl