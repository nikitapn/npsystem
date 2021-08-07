#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <deque>
#include <stack>
#include <thread>
#include <optional>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <openssl/sha.h>
#include <nplib/webserver/network.hpp>
#include <nplib/webserver/auth_system.hpp>
#include <nplib/webserver/ssl/server_certificate.hpp>


namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

using tcp = net::ip::tcp;
using error_code = boost::system::error_code;

static networking::Factory* g_factory;

namespace networking {

Config g_server_cfg;

class SessionImpl {
public:
	virtual void send(std::string&& data) = 0;
	virtual ~SessionImpl() = default;
};

template<typename WebSocket_Session>
class SessionImplT : public SessionImpl {
	std::weak_ptr<WebSocket_Session> ws_;
public:
	virtual void send(std::string&& data) {
		if (auto locked = ws_.lock()) {
			locked->write(std::move(data), std::move(locked));
		} else {
			// std::cerr << "websocket was closed...";
		}
	}

	SessionImplT(std::weak_ptr<WebSocket_Session> ws) : ws_(ws) {}
};

NPLIB_IMPORT_EXPORT void Session::send(std::string&& data) {
	impl_->send(std::move(data));
}

NPLIB_IMPORT_EXPORT Session::~Session() {
	delete impl_;
}

}
/*
class Pools {
	std::vector<std::unique_ptr<networking::NetPools>> pools_;
	std::stack<networking::NetPools*> reuse_;
	std::mutex mut_;
public:
	networking::NetPools* acquire() {
		std::lock_guard<std::mutex> lk(mut_);
		if (reuse_.empty() == false) {
			auto pool = reuse_.top();
			reuse_.pop();
			return pool;
		}
		pools_.push_back(std::make_unique<networking::NetPools>());
		return pools_.back().get();
	}
	void release(networking::NetPools* pool) {
		std::lock_guard<std::mutex> lk(mut_);
		reuse_.push(pool);
	}
} g_pools;
*/

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path)
{
	using beast::iequals;
	auto const ext = [&path]
	{
		auto const pos = path.rfind(".");
		if (pos == beast::string_view::npos)
			return beast::string_view{};
		return path.substr(pos);
	}();
	if (iequals(ext, ".htm"))  return "text/html";
	if (iequals(ext, ".html")) return "text/html";
	if (iequals(ext, ".php"))  return "text/html";
	if (iequals(ext, ".css"))  return "text/css";
	if (iequals(ext, ".txt"))  return "text/plain";
	if (iequals(ext, ".pdf"))  return "application/pdf";
	if (iequals(ext, ".js"))   return "application/javascript";
	if (iequals(ext, ".json")) return "application/json";
	if (iequals(ext, ".xml"))  return "application/xml";
	if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
	if (iequals(ext, ".flv"))  return "video/x-flv";
	if (iequals(ext, ".png"))  return "image/png";
	if (iequals(ext, ".jpe"))  return "image/jpeg";
	if (iequals(ext, ".jpeg")) return "image/jpeg";
	if (iequals(ext, ".jpg"))  return "image/jpeg";
	if (iequals(ext, ".gif"))  return "image/gif";
	if (iequals(ext, ".bmp"))  return "image/bmp";
	if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
	if (iequals(ext, ".tiff")) return "image/tiff";
	if (iequals(ext, ".tif"))  return "image/tiff";
	if (iequals(ext, ".svg"))  return "image/svg+xml";
	if (iequals(ext, ".svgz")) return "image/svg+xml";
	return "application/text";
}

std::optional<std::string> get_session_id(boost::string_view str) {
	constexpr auto field = boost::string_view("session_id=");
	if ((str.length() < field.length() + SHA256_DIGEST_LENGTH) ||
		(str.find(field) == boost::string_view::npos)) return {};
	return std::string(str.begin() + field.length(), str.begin() + field.length() + SHA256_DIGEST_LENGTH);
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
	beast::string_view base,
	beast::string_view path)
{
	if (base.empty())
		return std::string(path);
	std::string result(base);
#ifdef BOOST_MSVC
	char constexpr path_separator = '\\';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
	for (auto& c : result)
		if (c == '/')
			c = path_separator;
#else
	char constexpr path_separator = '/';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
#endif
	return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
	class Body, class Allocator,
	class Send>
	void
	handle_request(
		beast::string_view doc_root,
		http::request<Body, http::basic_fields<Allocator>>&& req,
		Send&& send)
{
	// Returns a bad request response
	auto const bad_request =
		[&req](beast::string_view why)
	{
		http::response<http::string_body> res{ http::status::bad_request, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();
		return res;
	};

	// Returns a not found response
	auto const not_found =
		[&req](beast::string_view target)
	{
		http::response<http::string_body> res{ http::status::not_found, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + std::string(target) + "' was not found.";
		res.prepare_payload();
		return res;
	};

	// Returns a server error response
	auto const server_error =
		[&req](beast::string_view what)
	{
		http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + std::string(what) + "'";
		res.prepare_payload();
		return res;
	};

	/*
	// Redirect to login
	auto const login_redirect = [&req, &doc_root]() {
		http::response<http::empty_body> res{
			http::status::found,
			req.version()
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::location, "/login.html");
		res.content_length(0);
		res.keep_alive(req.keep_alive());
		return res;
	};
	*/

	// send response in json
	auto const ajax_json_request = [&req](std::string val, std::string msg) {
		std::string json = "{ \"" + val + "\": \"" + msg + "\" }";
		auto size = json.size();
		http::response<http::string_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(json)),
			std::make_tuple(http::status::ok, req.version())
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "application/json");
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return res;
	};

	// Make sure we can handle the method
	if (req.method() != http::verb::get
		&& req.method() != http::verb::post
		&& req.method() != http::verb::head)
		return send(bad_request("Unknown HTTP-method"));

	if (networking::g_server_cfg.http_accept_upgrade_only) 
		return send(bad_request("Illegal request: only Upgrade is allowed"));
		
	// Request path must be absolute and not contain "..".
	if (req.target().empty() ||
		req.target()[0] != '/' ||
		req.target().find("..") != beast::string_view::npos)
		return send(bad_request("Illegal request-target"));

	std::cout << "target: " << req.target() << std::endl;

	bool guest_dir = (req.target().find("/3rd/") != boost::beast::string_view::npos);

	std::string path;

	if (guest_dir) {
		path = path_cat(doc_root, req.target());
	} else {
		// Build the path to the requested file
		auto cookie = req[http::field::cookie];
		bool registered = false;

		{
			auto session = get_session_id(cookie);
			if (session && auth::AuthSystem::get_mutable_instance().sessions.is_session_exist(*session)) {
				registered = true;
			}
		}

		if (registered) {
			path = path_cat(doc_root, req.target());
			if (req.target().length() == 1 && req.target().back() == '/') path.append("index.html");
		} else if (req.target() == "/login.html") {
			path = path_cat(doc_root, "/login.html");
		} else if (
			(req.target().length() == (sizeof("/reg") - 1)) &&
			(req.target().find("/reg") != boost::string_view::npos)
			) {
			std::string link;
			auto s = auth::AuthSystem::get_mutable_instance().register_player(req.body(), link);
			switch (s) {
			case auth::RegStatus::PLAYER_REGISTERED:
				return send(ajax_json_request("success", link));
			case auth::RegStatus::EMAIL_ALREADY_REGISTERED:
				return send(ajax_json_request("error", "player with this email is already registered"));
			case auth::RegStatus::INVALID_DATA:
				return send(ajax_json_request("error", "something went wrong"));
			case auth::RegStatus::DATABASE_ERROR:
				return send(ajax_json_request("error", "something went wrong"));
			default:
				assert(false);
				break;
			}
			assert(false);
		} else if (
			(req.target().length() == (sizeof("/log") - 1)) &&
			(req.target().find("/log") != boost::string_view::npos)
			) {
			std::string session_id;
			auto s = auth::AuthSystem::get_mutable_instance().login_player(req.body(), session_id);
			switch (s) {
			case auth::LogStatus::PLAYER_LOGGED:
				return send(ajax_json_request("success", session_id));
			case auth::LogStatus::EMAIL_NOT_EXIST:
				return send(ajax_json_request("error", "Player with this email is not registered"));
			case auth::LogStatus::WRONG_PASSWORD:
				return send(ajax_json_request("error", "Wrong password"));
			case auth::LogStatus::INVALID_DATA:
				return send(ajax_json_request("error", "something went wrong"));
			case auth::LogStatus::DATABASE_ERROR:
				return send(ajax_json_request("error", "something went wrong"));
			default:
				assert(false);
				break;
			}
			assert(false);
		} else {
			path = path_cat(doc_root, req.target());
			if (req.target().length() == 1 && req.target().back() == '/') path.append("index.html");
			// return send(login_redirect());
		}
	}

	// Attempt to open the file
	beast::error_code ec;
	http::file_body::value_type body;
	body.open(path.c_str(), beast::file_mode::scan, ec);

	// Handle the case where the file doesn't exist
	if (ec == beast::errc::no_such_file_or_directory)
		return send(not_found(req.target()));

	// Handle an unknown error
	if (ec)
		return send(server_error(ec.message()));

	// Cache the size since we need it after the move
	auto const size = body.size();

	// Respond to HEAD request
	if (req.method() == http::verb::head)
	{
		http::response<http::empty_body> res{ http::status::ok, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, mime_type(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return send(std::move(res));
	}

	// Respond to GET request
	http::response<http::file_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, req.version()) };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, mime_type(path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return send(std::move(res));
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
	// ssl::error::stream_truncated, also known as an SSL "short read",
	// indicates the peer closed the connection without performing the
	// required closing handshake (for example, Google does this to
	// improve performance). Generally this can be a security issue,
	// but if your communication protocol is self-terminated (as
	// it is with both HTTP and WebSocket) then you may simply
	// ignore the lack of close_notify.
	//
	// https://github.com/boostorg/beast/issues/38
	//
	// https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
	//
	// When a short read would cut off the end of an HTTP message,
	// Beast returns the error beast::http::error::partial_message.
	// Therefore, if we see a short read here, it has occurred
	// after the message has been completed, so it is safe to ignore it.

	if (ec == net::ssl::error::stream_truncated) return;

	std::cerr << what << ": " << ec.message() << "\n";
}

//------------------------------------------------------------------------------

// Echoes back all received WebSocket messages.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class websocket_session
{
	// Access the derived class, this is part of
	// the Curiously Recurring Template Pattern idiom.
	Derived& derived() {
		return static_cast<Derived&>(*this);
	}

	networking::net_buffer rx_buffer_;

	struct work {
		virtual ~work() = default;
		virtual void operator()() = 0;
	};

	std::deque<std::unique_ptr<work>> write_queue_;

	// Start the asynchronous operation
	template<class Body, class Allocator>
	void do_accept(http::request<Body, http::basic_fields<Allocator>> req) {
		// Set suggested timeout settings for the websocket
		derived().ws().set_option(
			websocket::stream_base::timeout::suggested(
				beast::role_type::server));

		// Set a decorator to change the Server of the handshake
		derived().ws().set_option(
			websocket::stream_base::decorator(
				[](websocket::response_type& res)
				{
					res.set(http::field::server,
						std::string(BOOST_BEAST_VERSION_STRING));
				}));

		// Accept the websocket handshake
		derived().ws().async_accept(
			req,
			beast::bind_front_handler(
				&websocket_session::on_accept,
				derived().shared_from_this()));
	}

	void on_accept(beast::error_code ec) {
		if (ec) return fail(ec, "accept");

		derived().create_connection();

		// Read a message
		do_read();
	}

	void do_read() {
		//std::cout << "do_read" << std::endl;
		// Read a message into our buffer
		derived().ws().async_read(
			rx_buffer_,
			beast::bind_front_handler(
				&websocket_session::on_read,
				derived().shared_from_this()));
	}

	void on_read(beast::error_code ec, std::size_t bytes_transferred) {
		//std::cout << "on_read" << std::endl;

		boost::ignore_unused(bytes_transferred);

		// This indicates that the websocket_session was closed
		if (ec == websocket::error::closed) return;

		if (ec) fail(ec, "read");

		struct work_impl : work {
			Derived& self_;
			std::string data_;

			work_impl(Derived& self, std::string&& data)
				: self_(self)
				, data_(std::move(data))
			{
			}

			virtual void operator()() final {
				self_.ws().text(false); // binary mode only
				self_.ws().async_write(
					net::buffer(data_),
					std::bind(
						&websocket_session::on_write,
						self_.shared_from_this(),
						std::placeholders::_1,
						std::placeholders::_2,
						true));
			}
		};

		auto response = connection_->handle_client_message(std::move(rx_buffer_));
		rx_buffer_.consume(rx_buffer_.size());
		
		if (response.size()) {
			write_queue_.push_back(
				std::unique_ptr<work>(new work_impl(
					derived(),
					std::move(response)
				)));
			if (write_queue_.size() == 1) {
				(*write_queue_.front())();
			}
		}
	}

	void on_write(beast::error_code ec, std::size_t bytes_transferred, bool is_answer) {
		//std::cout << "on_write" << std::endl;

		if (ec == websocket::error::closed) {
			// std::cout << "websocket::error::closed" << std::endl;
			return;
		}

		boost::ignore_unused(bytes_transferred);

		if (ec) return fail(ec, "write");

		assert(write_queue_.size() >= 1);
		write_queue_.pop_front();

		// Do another read
		if (is_answer) { // avoiding double async_read in sequence
			do_read();
		}

		if (!write_queue_.empty()) {
			(*write_queue_.front())();
		}
	}

	void write_in_this_strand(std::string&& data) {
		struct work_impl : work {
			Derived& self_;
			std::string data_;

			work_impl(Derived& self, std::string&& data)
				: self_(self)
				, data_(std::move(data))
			{
			}

			virtual void operator()() final {
				//std::cout << "write_in_this_strand()" << std::endl;
				self_.ws().text(false); // binary mode only
				self_.ws().async_write(
					net::buffer(data_),
					std::bind(
						&websocket_session::on_write,
						self_.shared_from_this(),
						std::placeholders::_1,
						std::placeholders::_2,
						false));
			}
		};

		write_queue_.push_back(
			std::unique_ptr<work>(new work_impl(derived(), std::move(data)))
		);

		if (write_queue_.size() == 1) {
			(*write_queue_.front())();
		}
	}
public:
	void write(std::string&& data, std::shared_ptr<Derived>&& self) {
		//std::cout << "write" << std::endl;

		auto sp = std::make_shared<std::string>(std::move(data));
		net::post(derived().ws().get_executor(),
			[self, sp]() {
				auto fn = std::mem_fn(&websocket_session::write_in_this_strand);
				return fn(std::move(self), std::move(*sp));
			}
			/*
			std::bind(
				&websocket_session::write_in_this_strand,
				std::move(self),
				data
			)*/
		);
	}

	// Start the asynchronous operation
	template<class Body, class Allocator>
	void run(http::request<Body, http::basic_fields<Allocator>> req) {
		// Accept the WebSocket upgrade request
		/*
		derived().ws().set_option(
            websocket::stream_base::decorator(
                [](http::response_header<> &hdr) {
                    hdr.set(
                        http::field::sec_websocket_protocol,
                        "binary");
                }));
		*/
		std::cout << "upgrade: " << req.target() << std::endl;
		
		do {
			if (req.target().length() <= 1) break;

			std::size_t index = req.target().find_last_of("/");
			if (index == std::string::npos || index >= req.target().length() - 1) break;
			
			auto session_id = std::string(req.target().substr(index + 1));

			std::cout << session_id << std::endl;

			auto user = auth::AuthSystem::get_mutable_instance().sessions.get_user(session_id);
			if (user) {
				session_data_.session_id = session_id;
				session_data_.user = (*user).user_name;
				session_data_.email = (*user).email;

				do_accept(std::move(req));

				return;
			}
		} while (0);

		session_data_.session_id = "";
		session_data_.user = "Guest";
		session_data_.email = "";

		do_accept(std::move(req));
	}

	websocket_session(std::shared_ptr<networking::shared_state>&& state/*, networking::NetPools& pools*/)
		: state_(std::move(state))
//		, pools_(pools)
//		, rx_buffer_(pools)
	{
	}

	~websocket_session() {
		//std::cout << "~websocket_session()" << std::endl;
		derived().remove_connection();
//		g_pools.release(&pools_);
	}
protected:
	networking::SessionData session_data_;
	std::shared_ptr<networking::shared_state> state_;
	networking::Session* connection_ = nullptr;
	std::vector<std::string> to_send_queue_;
	//networking::NetPools& pools_;
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_session
	: public websocket_session<plain_websocket_session>
	, public std::enable_shared_from_this<plain_websocket_session>
{
	using base = websocket_session<plain_websocket_session>;

	websocket::stream<beast::tcp_stream> ws_;

public:
	// Create the session
	explicit
		plain_websocket_session(
			beast::tcp_stream&& stream,
			std::shared_ptr<networking::shared_state>&& state)
		: websocket_session(std::move(state)/*, *g_pools.acquire()*/)
		, ws_(std::move(stream))
	{
	}

	// Called by the base class
	websocket::stream<beast::tcp_stream>&
		ws()
	{
		return ws_;
	}

	void create_connection() {
		auto session = g_factory->create_session(this->session_data_);
		session->impl_ = new networking::SessionImplT<plain_websocket_session>(weak_from_this());
		base::connection_ = session.get();
		base::state_->join(std::move(session));
	}

	void remove_connection() {
		base::state_->leave(base::connection_);
	}
};

//------------------------------------------------------------------------------

// Handles an SSL WebSocket connection
class ssl_websocket_session
	: public websocket_session<ssl_websocket_session>
	, public std::enable_shared_from_this<ssl_websocket_session> {
	using base = websocket_session<ssl_websocket_session>;

	websocket::stream<
		beast::ssl_stream<beast::tcp_stream>> ws_;
public:
	// Create the ssl_websocket_session
	explicit
		ssl_websocket_session(
			beast::ssl_stream<beast::tcp_stream>&& stream,
			std::shared_ptr<networking::shared_state>&& state)
		: websocket_session(std::move(state)/*, *g_pools.acquire()*/)
		, ws_(std::move(stream))
	{
	}

	// Called by the base class
	websocket::stream<
		beast::ssl_stream<beast::tcp_stream>>&
		ws()
	{
		return ws_;
	}

	void create_connection() {
		auto session = g_factory->create_session(this->session_data_);
		session->impl_ = new networking::SessionImplT<ssl_websocket_session>(weak_from_this());
		base::connection_ = session.get();
		base::state_->join(std::move(session));
	}

	void remove_connection() {
		base::state_->leave(base::connection_);
	}
};

//------------------------------------------------------------------------------

template<class Body, class Allocator>
void
make_websocket_session(
	beast::tcp_stream stream,
	http::request<Body, http::basic_fields<Allocator>> req,
	std::shared_ptr<networking::shared_state>&& state)
{
	std::make_shared<plain_websocket_session>(
		std::move(stream),
		std::move(state)
		)->run(std::move(req));
}

template<class Body, class Allocator>
void
make_websocket_session(
	beast::ssl_stream<beast::tcp_stream> stream,
	http::request<Body, http::basic_fields<Allocator>> req,
	std::shared_ptr<networking::shared_state>&& state)
{
	std::make_shared<ssl_websocket_session>(
		std::move(stream),
		std::move(state)
		)->run(std::move(req));
}

//------------------------------------------------------------------------------

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class http_session
{
	// Access the derived class, this is part of
	// the Curiously Recurring Template Pattern idiom.
	Derived& derived() {
		return static_cast<Derived&>(*this);
	}

	// This queue is used for HTTP pipelining.
	class queue
	{
		enum
		{
			// Maximum number of responses we will queue
			limit = 8
		};

		// The type-erased, saved work item
		struct work
		{
			virtual ~work() = default;
			virtual void operator()() = 0;
		};

		http_session& self_;
		std::vector<std::unique_ptr<work>> items_;

	public:
		explicit queue(http_session& self)
			: self_(self) {
			static_assert(limit > 0, "queue limit must be positive");
			items_.reserve(limit);
		}

		// Returns `true` if we have reached the queue limit
		bool is_full() const {
			return items_.size() >= limit;
		}

		// Called when a message finishes sending
		// Returns `true` if the caller should initiate a read
		bool on_write() {
			BOOST_ASSERT(!items_.empty());
			auto const was_full = is_full();
			items_.erase(items_.begin());
			if (!items_.empty()) (*items_.front())();
			return was_full;
		}

		// Called by the HTTP handler to send a response.
		template<bool isRequest, class Body, class Fields>
		void operator()(http::message<isRequest, Body, Fields>&& msg) {
			// This holds a work item
			struct work_impl : work {
				http_session& self_;
				http::message<isRequest, Body, Fields> msg_;

				work_impl(
					http_session& self,
					http::message<isRequest, Body, Fields>&& msg)
					: self_(self)
					, msg_(std::move(msg))
				{
				}

				void operator()() {
					http::async_write(
						self_.derived().stream(),
						msg_,
						beast::bind_front_handler(
							&http_session::on_write,
							self_.derived().shared_from_this(),
							msg_.need_eof()));
				}
			};

			// Allocate and store the work
			items_.push_back(
				boost::make_unique<work_impl>(self_, std::move(msg))
			);

			// If there was no previous work, start this one
			if (items_.size() == 1) (*items_.front())();
		}
	};

	std::shared_ptr<std::string const> doc_root_;
	queue queue_;
	// The parser is stored in an optional container so we can
	// construct it from scratch it at the beginning of each new message.
	boost::optional<http::request_parser<http::string_body>> parser_;

protected:
	beast::flat_buffer buffer_;
	std::shared_ptr<networking::shared_state> state_;
public:
	// Construct the session
	http_session(beast::flat_buffer buffer,
		std::shared_ptr<std::string const> const& doc_root,
		std::shared_ptr<networking::shared_state> state
	)
		: doc_root_(doc_root)
		, queue_(*this)
		, buffer_(std::move(buffer))
		, state_(state)
	{
	}

	void do_read()
	{
		// Construct a new parser for each message
		parser_.emplace();

		// Apply a reasonable limit to the allowed size
		// of the body in bytes to prevent abuse.
		parser_->body_limit(10000);

		// Set the timeout.
		beast::get_lowest_layer(
			derived().stream()
		).expires_after(std::chrono::seconds(30));

		// Read a request using the parser-oriented interface
		http::async_read(
			derived().stream(),
			buffer_,
			*parser_,
			beast::bind_front_handler(
				&http_session::on_read,
				derived().shared_from_this()));
	}

	void on_read(beast::error_code ec, std::size_t bytes_transferred) {
		boost::ignore_unused(bytes_transferred);

		// This means they closed the connection
		if (ec == http::error::end_of_stream)
			return derived().do_eof();

		if (ec) return fail(ec, "read");

		// See if it is a WebSocket Upgrade
		if (websocket::is_upgrade(parser_->get()))
		{
			// Disable the timeout.
			// The websocket::stream uses its own timeout settings.
			beast::get_lowest_layer(derived().stream()).expires_never();

			// Create a websocket session, transferring ownership
			// of both the socket and the HTTP request.
			return make_websocket_session(
				derived().release_stream(),
				parser_->release(),
				std::move(state_));
		}
		
		// Send the response
		handle_request(*doc_root_, parser_->release(), queue_);

		// If we aren't at the queue limit, try to pipeline another request
		if (!queue_.is_full()) do_read();

		/*
		// a function to select the most preferred protocol from a comma-separated list
auto select_protocol = [](string_view offered_tokens) -> std::string
{
    // tokenize the Sec-Websocket-Protocol header offered by the client
    http::token_list offered( offered_tokens );

    // an array of protocols supported by this server
    // in descending order of preference
    static const std::array<string_view, 3>
        supported = {
        "v3.my.chat",
        "v2.my.chat",
        "v1.my.chat"
    };

    std::string result;

    for (auto proto : supported)
    {
        auto iter = std::find(offered.begin(), offered.end(), proto);
        if (iter != offered.end())
        {
            // we found a supported protocol in the list offered by the client
            result.assign(proto.begin(), proto.end());
            break;
        }
    }

    return result;
};


// This buffer is required for reading HTTP messages
flat_buffer buffer;

// Read the HTTP request ourselves
http::request<http::string_body> req;
http::read(sock, buffer, req);

// See if it's a WebSocket upgrade request
if(websocket::is_upgrade(req))
{
    // we store the selected protocol in a std::string here because
    // we intend to capture it in the decorator's lambda below
    std::string protocol =
        select_protocol(
            req[http::field::sec_websocket_protocol]);

    if (protocol.empty())
    {
        // none of our supported protocols were offered
        http::response<http::string_body> res;
        res.result(http::status::bad_request);
        res.body() = "No valid sub-protocol was offered."
                      " This server implements"
                      " v3.my.chat,"
                      " v2.my.chat"
                      " and v1.my.chat";
        http::write(sock, res);
    }
    else
    {
        // Construct the stream, transferring ownership of the socket
        stream<tcp_stream> ws(std::move(sock));

        ws.set_option(
            stream_base::decorator(
                [protocol](http::response_header<> &hdr) {
                    hdr.set(
                        http::field::sec_websocket_protocol,
                        protocol);
                }));

        // Accept the upgrade request
        ws.accept(req);
    }
}
else
{
    // Its not a WebSocket upgrade, so
    // handle it like a normal HTTP request.
}
*/
	}

	void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred) {
		boost::ignore_unused(bytes_transferred);

		if (ec) return fail(ec, "write");

		if (close) {
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return derived().do_eof();
		}

		// Inform the queue that a write completed
		if (queue_.on_write()) {
			// Read another request
			do_read();
		}
	}
};

//------------------------------------------------------------------------------

// Handles a plain HTTP connection
class plain_http_session
	: public http_session<plain_http_session>
	, public std::enable_shared_from_this<plain_http_session>
{
	beast::tcp_stream stream_;

public:
	// Create the session
	plain_http_session(
		beast::tcp_stream&& stream,
		beast::flat_buffer&& buffer,
		std::shared_ptr<std::string const> const& doc_root,
		std::shared_ptr<networking::shared_state> state)
		: http_session<plain_http_session>(std::move(buffer), doc_root, state)
		, stream_(std::move(stream))
	{
	}

	// Start the session
	void run() {
		this->do_read();
	}

	// Called by the base class
	beast::tcp_stream& stream() {
		return stream_;
	}

	// Called by the base class
	beast::tcp_stream release_stream() {
		return std::move(stream_);
	}

	// Called by the base class
	void do_eof()
	{
		// Send a TCP shutdown
		beast::error_code ec;
		stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
};

//------------------------------------------------------------------------------

// Handles an SSL HTTP connection
class ssl_http_session
	: public http_session<ssl_http_session>
	, public std::enable_shared_from_this<ssl_http_session>
{
	beast::ssl_stream<beast::tcp_stream> stream_;
public:
	// Create the http_session
	ssl_http_session(
		beast::tcp_stream&& stream,
		ssl::context& ctx,
		beast::flat_buffer&& buffer,
		std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<networking::shared_state> state)
		: http_session<ssl_http_session>(std::move(buffer), doc_root, state)
		, stream_(std::move(stream), ctx)
	{
	}

	// Start the session
	void
		run()
	{
		// Set the timeout.
		beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

		// Perform the SSL handshake
		// Note, this is the buffered version of the handshake.
		stream_.async_handshake(
			ssl::stream_base::server,
			buffer_.data(),
			beast::bind_front_handler(
				&ssl_http_session::on_handshake,
				shared_from_this()));
	}

	// Called by the base class
	beast::ssl_stream<beast::tcp_stream>&
		stream()
	{
		return stream_;
	}

	// Called by the base class
	beast::ssl_stream<beast::tcp_stream>
		release_stream()
	{
		return std::move(stream_);
	}

	// Called by the base class
	void
		do_eof()
	{
		// Set the timeout.
		beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

		// Perform the SSL shutdown
		stream_.async_shutdown(
			beast::bind_front_handler(
				&ssl_http_session::on_shutdown,
				shared_from_this()));
	}

private:
	void
		on_handshake(
			beast::error_code ec,
			std::size_t bytes_used)
	{
		if (ec)
			return fail(ec, "handshake");

		// Consume the portion of the buffer used by the handshake
		buffer_.consume(bytes_used);

		do_read();
	}

	void
		on_shutdown(beast::error_code ec)
	{
		if (ec)
			return fail(ec, "shutdown");

		// At this point the connection is closed gracefully
	}
};

//------------------------------------------------------------------------------

// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session>
{
	beast::tcp_stream stream_;
	ssl::context& ctx_;
	std::shared_ptr<std::string const> doc_root_;
	beast::flat_buffer buffer_;
	std::shared_ptr<networking::shared_state> state_;
public:
	explicit
		detect_session(
			tcp::socket&& socket,
			ssl::context& ctx,
			std::shared_ptr<std::string const> const& doc_root,
			std::shared_ptr<networking::shared_state> state)
		: stream_(std::move(socket))
		, ctx_(ctx)
		, doc_root_(doc_root)
		, state_(state)
	{
	}

	// Launch the detector
	void
		run()
	{
		// Set the timeout.
		stream_.expires_after(std::chrono::seconds(30));

		beast::async_detect_ssl(
			stream_,
			buffer_,
			beast::bind_front_handler(
				&detect_session::on_detect,
				this->shared_from_this()));
	}

	void
		on_detect(beast::error_code ec, bool result)
	{
		if (ec)
			return fail(ec, "detect");

		if (result)
		{
			// Launch SSL session
			std::make_shared<ssl_http_session>(
				std::move(stream_),
				ctx_,
				std::move(buffer_),
				doc_root_,
				state_)->run();
			return;
		}

		// Launch plain session
		std::make_shared<plain_http_session>(
			std::move(stream_),
			std::move(buffer_),
			doc_root_,
			state_)->run();
	}
};

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
	net::io_context& ioc_;
	ssl::context& ctx_;
	tcp::acceptor acceptor_;
	std::shared_ptr<std::string const> doc_root_;
	std::shared_ptr<networking::shared_state> state_;
public:
	listener(
		net::io_context& ioc,
		ssl::context& ctx,
		tcp::endpoint endpoint,
		std::shared_ptr<std::string const> const& doc_root,
		std::shared_ptr<networking::shared_state> const& state)
		: ioc_(ioc)
		, ctx_(ctx)
		, acceptor_(net::make_strand(ioc))
		, doc_root_(doc_root)
		, state_(state)
	{
		beast::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec)
		{
			fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(
			net::socket_base::max_listen_connections, ec);
		if (ec)
		{
			fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void
		run()
	{
		do_accept();
	}

private:
	void do_accept() {
		// The new connection gets its own strand
		acceptor_.async_accept(
			net::make_strand(ioc_),
			beast::bind_front_handler(
				&listener::on_accept,
				shared_from_this()));
	}

	void on_accept(beast::error_code ec, tcp::socket socket) {
		if (ec) {
			fail(ec, "accept");
		} else {
			// Create the detector http_session and run it
			std::make_shared<detect_session>(
				std::move(socket),
				ctx_,
				doc_root_,
				state_)->run();

			/*
			std::make_shared<plain_http_session>(
				std::move(socket),
				std::move(buffer_),
				doc_root_,
			state_)->run();
			*/

		}
		// Accept another connection
		do_accept();
	}
};

extern void open_users_db();

namespace networking {
// The SSL context is required, and holds certificates
ssl::context ctx{ ssl::context::tlsv12 };

NPLIB_IMPORT_EXPORT void shared_state::join(std::unique_ptr<Session> session) {
	std::lock_guard<std::mutex> lk(mut_);
	cons_.push_back(std::move(session));
}

NPLIB_IMPORT_EXPORT void shared_state::leave(Session* session) {
	if (!session) return;
	std::lock_guard<std::mutex> lk(mut_);
	auto founded = std::find_if(cons_.begin(), cons_.end(), [session](auto& uptr) { return uptr.get() == session; });
	assert(founded != cons_.end());
	cons_.erase(founded);
}

NPLIB_IMPORT_EXPORT void shared_state::send_all(std::string data) {
	std::lock_guard<std::mutex> lk(mut_);
	for (auto& con : cons_) {
		con->send(std::string(data));
	}
}
NPLIB_IMPORT_EXPORT void shared_state::send_all_except_one(std::string data, std::shared_ptr<Session>& except) {
	std::lock_guard<std::mutex> lk(mut_);
	for (auto& con : cons_) {
		if (con->id() != except->id()) con->send(std::string(data));
	}
}

NPLIB_IMPORT_EXPORT void init(
	Config&& cfg,
	boost::asio::io_context& ioc,
	std::shared_ptr<shared_state>& state,
	Factory* factory,
	std::string address,
	unsigned short port,
	std::string_view doc_root)
{
	assert(factory);
	g_factory = factory;

	g_server_cfg = std::move(cfg);

	open_users_db();

	load_server_certificate(ctx);
	
	// Create and launch a listening port
	std::make_shared<listener>(ioc, ctx,
		tcp::endpoint{ net::ip::make_address(address), port },
		std::make_shared<std::string const>(doc_root),
		state
		)->run();

	std::thread([&ioc]() { ioc.run(); }).detach();
}

} // namespace networking


