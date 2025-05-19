#pragma once

#include <nprpc/impl/session.hpp>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <deque>

namespace nprpc::impl {

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

using tcp = net::ip::tcp;
using error_code = boost::system::error_code;

using tcp_stream_strand = net::basic_stream_socket<
  net::ip::tcp, net::strand<net::io_context::executor_type>>;

using beast_tcp_stream_strand = beast::basic_stream<
  net::ip::tcp, net::strand<net::io_context::executor_type>>;

// Report a failure
void beast_fail(beast::error_code ec, char const *what);

template <class Derived>
class WebSocketSession
  : public Session
{
  struct work {
    virtual void operator()() = 0;
    virtual void on_failed(const boost::system::error_code &ec) noexcept = 0;
    virtual void on_executed(flat_buffer &&rx_buffer) noexcept = 0;
    virtual ~work() = default;
  };

  std::deque<std::shared_ptr<work>> requests_;
  std::deque<std::unique_ptr<work>> answers_;

  bool reading_ = false;
  bool writing_ = false;
  bool request_sended_waiting_for_answer_ = false;

  Derived &derived() { return static_cast<Derived &>(*this); }

  void on_read(beast::error_code ec, std::size_t bytes_transferred);
  void on_write(
    beast::error_code ec,
    std::size_t bytes_transferred,
    bool is_answer);
  void add_request(std::shared_ptr<work> w);
protected:
  void do_read();
  void close();
  virtual void timeout_action() override final;
public:

  virtual void send_receive(
      flat_buffer &buffer,
      uint32_t timeout_ms);

  virtual void send_receive_async(
      flat_buffer &&buffer,
      std::optional<std::function<
        void(
          const boost::system::error_code &,
          flat_buffer &)>> &&completion_handler,
      uint32_t timeout_ms);

  WebSocketSession(net::any_io_executor executor)
    : Session(executor) {}
};

class PlainWebSocketSession
  : public WebSocketSession<PlainWebSocketSession>
  , public std::enable_shared_from_this<PlainWebSocketSession>
{
  using base = WebSocketSession<PlainWebSocketSession>;
  using websocket_t = websocket::stream<beast_tcp_stream_strand>;
  
  websocket_t ws_;
public:
  using stream_t = beast_tcp_stream_strand;
  // Create the session
  explicit PlainWebSocketSession(beast_tcp_stream_strand &&stream)
    : WebSocketSession<PlainWebSocketSession>(stream.get_executor())
    , ws_(std::move(stream)) 
    {
      auto endpoint = ws().next_layer().socket().remote_endpoint();
      ctx_.remote_endpoint = EndPoint(
        EndPointType::TcpTethered,
        endpoint.address().to_v4().to_string(), 
        endpoint.port());
  }

  // Called by the base class
  websocket_t &ws() { return ws_; }
};

class SSLWebSocketSession
  : public WebSocketSession<SSLWebSocketSession>
  , public std::enable_shared_from_this<SSLWebSocketSession>
{
  using base = WebSocketSession<SSLWebSocketSession>;
  using websocket_t = websocket::stream<
    beast::ssl_stream<beast_tcp_stream_strand>>;

  websocket_t ws_;
public:
  using stream_t = beast::ssl_stream<beast_tcp_stream_strand>;
  // Create the session
  explicit SSLWebSocketSession(beast::ssl_stream<beast_tcp_stream_strand>&& stream)
    : WebSocketSession<SSLWebSocketSession>(stream.get_executor())
    , ws_(std::move(stream)) 
  {
    auto endpoint = ws().next_layer().next_layer().socket().remote_endpoint();
    ctx_.remote_endpoint = EndPoint(
      EndPointType::TcpTethered,
      endpoint.address().to_v4().to_string(), 
      endpoint.port());
  }

  // Called by the base class
  websocket_t &ws() { return ws_; }
};

template <class Body, class Allocator>
void make_accepting_websocket_session(
  beast_tcp_stream_strand stream,
  http::request<Body, http::basic_fields<Allocator>> req);

template <class Body, class Allocator>
void make_accepting_websocket_session(
  beast::ssl_stream<beast_tcp_stream_strand> stream,
  http::request<Body, http::basic_fields<Allocator>> req);

} // namespace nprpc::impl
