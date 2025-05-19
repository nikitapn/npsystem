#include <nprpc/impl/nprpc_impl.hpp>
#include <nprpc/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <future>
#include <memory>
#include "helper.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;

namespace nprpc::impl {

// SSL Context factory
inline ssl::context make_ssl_context() {
  ssl::context ctx{ssl::context::tlsv12_client};
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);
  return ctx;
}

template<typename StreamType>
void WebSocketConnectionImpl<StreamType>::timeout_action() {
  boost::system::error_code ec;
  beast::get_lowest_layer(ws_).socket().cancel(ec);
  if (ec) fail(ec, "socket::cancel()");
}

template<typename StreamType>
void WebSocketConnectionImpl<StreamType>::reconnect() {
  /*
  beast::error_code ec;
  auto endpoint = tcp::endpoint(
  net::ip::address_v4(this->ctx_.remote_endpoint.hostname()), 
  this->ctx_.remote_endpoint.port());

  if (is_ssl_) {
  // Recreate SSL websocket
  ws_.ssl_ws.~ssl_stream();
  new (&ws_.ssl_ws) ssl_stream(ws_.ssl_ws.get_executor(), *ssl_ctx_);
  
  // Connect TCP
  beast::get_lowest_layer(ws_.ssl_ws).connect(endpoint, ec);
  if (ec) {
    close();
    throw nprpc::ExceptionCommFailure();
  }

  // SSL Handshake
  ws_.ssl_ws.next_layer().handshake(ssl::stream_base::client, ec);
  if (ec) {
    close();
    throw nprpc::ExceptionCommFailure();
  }

  // WebSocket Handshake
  ws_.ssl_ws.handshake(this->ctx_.remote_endpoint.hostname(), "/", ec);
  } else {
  // Recreate plain websocket
  ws_.plain_ws.~plain_stream();
  new (&ws_.plain_ws) plain_stream(ws_.plain_ws.get_executor());
  
  // Connect TCP
  beast::get_lowest_layer(ws_.plain_ws).connect(endpoint, ec);
  if (ec) {
    close();
    throw nprpc::ExceptionCommFailure();
  }

  // WebSocket Handshake
  ws_.plain_ws.handshake(this->ctx_.remote_endpoint.hostname(), "/", ec);
  }
  
  if (ec) {
  close();
  throw nprpc::ExceptionCommFailure();
  }*/
}

template<typename StreamType>
void WebSocketConnectionImpl<StreamType>::read_message() {
  timeout_timer_.expires_from_now(timeout_);
  read_buffer_.clear();

  // websocket only requires one singe asynchronous read operation
  // since the protocol itself handles message framing
  async_read_impl(ws_, [this](
    const boost::system::error_code& ec, size_t bytes_transferred)
  {
    timeout_timer_.expires_at(boost::posix_time::pos_infin);

    if (ec) {
      fail(ec, "read");
      (*this->wq_.front()).on_failed(ec);
      this->pop_and_execute_next_task();
      return;
    }

    auto& buf = this->current_rx_buffer();
    buf.consume(buf.size());
    auto mb = buf.prepare(bytes_transferred);
    std::memcpy(mb.data(), read_buffer_.data().data(), bytes_transferred);
    buf.commit(bytes_transferred);

    (*this->wq_.front()).on_executed();
    this->pop_and_execute_next_task();
  });
}

template<typename StreamType>
void WebSocketConnectionImpl<StreamType>::send_receive(
    flat_buffer& buffer,
    uint32_t timeout_ms)
{
  assert(*(uint32_t*)buffer.data().data() == buffer.size() - 4);

  if (g_cfg.debug_level >= DebugLevel::DebugLevel_EveryMessageContent) {
    dump_message(buffer, false);
  }

  struct WorkImpl : IOWork {
    flat_buffer& buf;
    WebSocketConnectionImpl<StreamType>& this_;
    uint32_t timeout_ms;
    std::promise<boost::system::error_code> promise;

    void operator()() noexcept override {
      this_.set_timeout(timeout_ms);
      this_.async_write_impl(this_.ws_, buf,
        [&](const boost::system::error_code& ec, size_t bytes_transferred) {
          boost::ignore_unused(bytes_transferred);
          if (ec) {
            on_failed(ec);
            this_.pop_and_execute_next_task();
            return;
          }
          this_.read_message();
        });
    }

    void on_failed(const boost::system::error_code& ec) noexcept override {
      promise.set_value(ec);
    }

    void on_executed() noexcept override {
      promise.set_value({});
    }

    flat_buffer& buffer() noexcept override { return buf; }
    std::future<boost::system::error_code> get_future() { return promise.get_future(); }

    WorkImpl(flat_buffer& _buf, WebSocketConnectionImpl<StreamType>& _this_, uint32_t _timeout_ms)
      : buf(_buf), this_(_this_), timeout_ms(_timeout_ms) {}
  };

  auto post_work_and_wait = [&]() -> boost::system::error_code {
    auto w = std::make_unique<WorkImpl>(buffer, *this, timeout_ms);
    auto fut = w->get_future();
    this->add_work(std::move(w));
    return fut.get();
  };

  auto ec = post_work_and_wait();
  
  if (!ec) {
    if (g_cfg.debug_level >= DebugLevel::DebugLevel_EveryMessageContent) {
      dump_message(buffer, true);
    }
    return;
  }

  if (ec == websocket::error::closed || ec == boost::asio::error::connection_reset) {
    reconnect();
    ec = post_work_and_wait();
    if (ec) {
      close();
      throw nprpc::ExceptionCommFailure();
    }
  } else {
    fail(ec, "send_receive");
    close();
    throw nprpc::ExceptionCommFailure();
  }
}

template<typename StreamType>
void WebSocketConnectionImpl<StreamType>::send_receive_async(
  flat_buffer&& buffer,
  std::optional<std::function<void(const boost::system::error_code&, flat_buffer&)>>&& completion_handler,
  uint32_t timeout_ms)
{

  assert(*(uint32_t*)buffer.data().data() == buffer.size() - 4);

  struct WorkImpl : IOWork {
    flat_buffer buf;
    WebSocketConnectionImpl<StreamType>& this_;
    uint32_t timeout_ms;
    std::optional<std::function<void(const boost::system::error_code&, flat_buffer&)>> handler;
    
    void operator()() noexcept override {
      this_.set_timeout(timeout_ms);
      auto write_handler = [&](const boost::system::error_code& ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) {
          on_failed(ec);
          this_.pop_and_execute_next_task();
          return;
        }
        this_.read_message();
      };

      this_.async_write_impl(this_.ws_, buf, write_handler);
    }

    void on_failed(const boost::system::error_code& ec) noexcept override {
      if (handler) handler.value()(ec, buf);
    }

    void on_executed() noexcept override {
      if (handler) handler.value()(boost::system::error_code{}, buf);
    }

    flat_buffer& buffer() noexcept override { return buf; }

    WorkImpl(flat_buffer&& _buf, 
      WebSocketConnectionImpl<StreamType>& _this_, 
      std::optional<std::function<void(const boost::system::error_code&, flat_buffer&)>>&& _handler,
      uint32_t _timeout_ms)
      : buf(std::move(_buf))
      , this_(_this_)
      , timeout_ms(_timeout_ms)
      , handler(std::move(_handler)) 
    {}
  };

  this->add_work(std::make_unique<WorkImpl>(std::move(buffer), *this, std::move(completion_handler), timeout_ms));
}

// Specialized constructors for SSL and plain streams
template<>
WebSocketConnectionImpl<ws_ssl_stream>::WebSocketConnectionImpl(const EndPoint& endpoint, boost::asio::ip::tcp::socket&& socket)
  : Session(socket.get_executor())
  , ws_(socket.get_executor(), *std::make_unique<ssl::context>(make_ssl_context())) {

  ctx_.remote_endpoint = endpoint;
  timeout_timer_.expires_at(boost::posix_time::pos_infin);

  beast::error_code ec;
  // Take ownership of the socket
  beast::get_lowest_layer(ws_).socket() = std::move(socket);

  // SSL Handshake
  ws_.next_layer().handshake(ssl::stream_base::client, ec);
  if (ec) throw nprpc::Exception(("SSL handshake failed: " + ec.message()).c_str());

  // WebSocket Handshake
  ws_.handshake(endpoint.hostname(), "/", ec);
  if (ec) throw nprpc::Exception(("Could not perform WebSocket handshake: " + ec.message()).c_str());

  start_timeout_timer();
}

template<>
WebSocketConnectionImpl<ws_plain_stream>::WebSocketConnectionImpl(
    const EndPoint& endpoint, boost::asio::ip::tcp::socket&& socket)
  : Session(socket.get_executor())
  , ws_(socket.get_executor()) {

  ctx_.remote_endpoint = endpoint;
  timeout_timer_.expires_at(boost::posix_time::pos_infin);

  beast::error_code ec;
  // Take ownership of the socket
  auto& sock = (beast::get_lowest_layer(ws_).socket() = std::move(socket));

  // Connect
  endpoint_ = sync_socket_connect(endpoint, sock);

  // Turn off the timeout on the tcp_stream, because
  // the websocket stream has its own timeout system.
  beast::get_lowest_layer(ws_).expires_never();

  // Set suggested timeout settings for the websocket
  ws_.set_option(
    websocket::stream_base::timeout::suggested(
      beast::role_type::client));

  // Set a decorator to change the User-Agent of the handshake
  ws_.set_option(websocket::stream_base::decorator(
    [](websocket::request_type& req) {
      req.set(boost::beast::http::field::user_agent,
        std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
    }));

  // Set binary message type
  ws_.text(false);

  // WebSocket Handshake
  ws_.handshake(endpoint.get_full(), "/", ec);
  if (ec) throw nprpc::Exception(("Could not perform WebSocket handshake: " + ec.message()).c_str());

  start_timeout_timer();
}

template<typename StreamType>
WebSocketConnectionImpl<StreamType>::~WebSocketConnectionImpl() {
  std::cout << "WebSocketConnectionImpl destructor called" << std::endl;
}

// Explicit template instantiations
template class WebSocketConnectionImpl<ws_plain_stream>;
template class WebSocketConnectionImpl<ws_ssl_stream>;

} // namespace nprpc::impl
