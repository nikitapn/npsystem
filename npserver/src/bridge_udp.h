#pragma once

#include <functional>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "bridge.h"

class bridge_udp 
	: public bridge
	, public boost::noncopyable {

	boost::asio::io_context ioc_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::deadline_timer deadline_;

	boost::posix_time::time_duration timeout_;
	boost::asio::ip::udp::endpoint endpoint_;
	
	void check_deadline() noexcept;
	std::size_t receive(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) noexcept; 
	static void handle_receive(const boost::system::error_code& ec, std::size_t length,
		boost::system::error_code* out_ec, std::size_t* out_length) noexcept;
public:
	bridge_udp(std::string remote_ip, uint16_t port, uint64_t timeout_ms);
	virtual int send_recive(const frame &output, frame &recived, size_t expected_length) noexcept override;
};