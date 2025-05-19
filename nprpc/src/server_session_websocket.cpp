// Based on libs/beast/example/advanced/server-flex/advanced_server_flex.cpp

// #define BOOST_ASIO_NO_DEPRECATED

#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <string>
#include <future>
#include <deque>
#include <optional>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include <nprpc/impl/nprpc_impl.hpp>
#include <nprpc/impl/session.hpp>
#include <nprpc/nprpc_base.hpp>
#include <nprpc/impl/websocket_session.hpp>

namespace nprpc::impl {

template <class T>
concept AnyWebSocketSession = 
  std::is_base_of_v<PlainWebSocketSession, T> ||
  std::is_base_of_v<SSLWebSocketSession, T>;

template <AnyWebSocketSession Derived>
class websocket_session_with_acceptor : public Derived
{ 
  // Start accepting handshake for the WebSocket session.
  template <class Body, class Allocator>
  void do_accept(http::request<Body, http::basic_fields<Allocator>> req) {
    // Set suggested timeout settings for the websocket
    this->ws().set_option(
      websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    this->ws().set_option(
      websocket::stream_base::decorator(
        [](websocket::response_type &res) {
          res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING));
        }));

    websocket::permessage_deflate opt;
    opt.client_enable = true;
    opt.server_enable = true;
    this->ws().set_option(opt);

    // Accept the websocket handshake
    this->ws().async_accept(
      req,
      beast::bind_front_handler(
        &websocket_session_with_acceptor::on_accept,
        std::static_pointer_cast<
          websocket_session_with_acceptor<Derived>>(this->shared_from_this())
    ));
  }

  void on_accept(beast::error_code ec) {
    if (ec) {
      this->close();
      return beast_fail(ec, "accept");
    }
    this->do_read();
  }
public:
  // Start the asynchronous operation
  template <class Body, class Allocator>
  void run(http::request<Body, http::basic_fields<Allocator>> req) {
    g_orb->add_connection(this->shared_from_this());
    // Accept the WebSocket upgrade request
    do_accept(std::move(req));
  }

  explicit websocket_session_with_acceptor(typename Derived::stream_t&& stream)
    : Derived(std::move(stream)) {}
};

template <>
void make_accepting_websocket_session(
  beast_tcp_stream_strand stream,
  http::request<http::string_body, http::basic_fields<std::allocator<char>>> req) 
{
  std::make_shared<
    websocket_session_with_acceptor<PlainWebSocketSession>>(
      std::move(stream))->run(std::move(req));
}

template <>
void make_accepting_websocket_session(
  beast::ssl_stream<beast_tcp_stream_strand> stream,
  http::request<http::string_body, http::basic_fields<std::allocator<char>>> req)
{
  std::make_shared<
    websocket_session_with_acceptor<SSLWebSocketSession>>(
      std::move(stream))->run(std::move(req));
}

} // namespace nprpc::impl
