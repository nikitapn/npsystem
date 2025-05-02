// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <boost/asio.hpp>
#include "bridge_udp.h"
#include "server.h"
#include "config.h"

namespace asio = boost::asio;

bridge_udp::bridge_udp(std::string remote_ip, uint16_t port, uint64_t timeout_ms) :
	socket_(ioc_, asio::ip::udp::v4()),
	deadline_(ioc_),
	timeout_(boost::posix_time::milliseconds(timeout_ms))
{
	endpoint_ = asio::ip::udp::endpoint(asio::ip::make_address_v4(remote_ip), port);
	// No deadline is required until the first socket operation is started. We
	// set the deadline to positive infinity so that the actor takes no action
	// until a specific deadline is set.
	deadline_.expires_at(boost::posix_time::pos_infin);

	// Start the persistent actor that checks for deadline expiry.
	check_deadline();
}

std::size_t bridge_udp::receive(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) noexcept {
	// Set a deadline for the asynchronous operation.
	deadline_.expires_from_now(timeout_);

	// Set up the variables that receive the result of the asynchronous
	// operation. The error code is set to would_block to signal that the
	// operation is incomplete. Asio guarantees that its asynchronous
	// operations will never fail with would_block, so any other value in
	// ec indicates completion.
	ec = boost::asio::error::would_block;
	std::size_t length = 0;

	// Start the asynchronous operation itself. The handle_receive function
	// used as a callback will update the ec and length variables.
	socket_.async_receive(boost::asio::buffer(buffer),
		std::bind(&bridge_udp::handle_receive, std::placeholders::_1, std::placeholders::_2, &ec, &length));

	// Block until the asynchronous operation has completed.
	do ioc_.run_one(); while (ec == boost::asio::error::would_block);

	return length;
}

void bridge_udp::check_deadline() noexcept {
	// Check whether the deadline has passed. We compare the deadline against
	// the current time since a new asynchronous operation may have moved the
	// deadline before this actor had a chance to run.
	if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
	{
		// The deadline has passed. The outstanding asynchronous operation needs
		// to be cancelled so that the blocked receive() function will return.
		//
		// Please note that cancel() has portability issues on some versions of
		// Microsoft Windows, and it may be necessary to use close() instead.
		// Consult the documentation for cancel() for further information.
		socket_.cancel();

		// There is no longer an active deadline. The expiry is set to positive
		// infinity so that the actor takes no action until a new deadline is set.
		deadline_.expires_at(boost::posix_time::pos_infin);
	}

	// Put the actor back to sleep.
	deadline_.async_wait(boost::bind(&bridge_udp::check_deadline, this));
}

void bridge_udp::handle_receive(const boost::system::error_code& ec, std::size_t length,
	boost::system::error_code* out_ec, std::size_t* out_length) noexcept {
	*out_ec = ec;
	*out_length = length;
}

int bridge_udp::send_recive(const frame &output, frame &recived, size_t expected_length) noexcept {
	try {
		socket_.send_to(asio::buffer(output), endpoint_);
	} catch (boost::system::system_error& ex) {
		std::cerr << ex.what() << '\n';
		return S_COMMUNICATION_ERROR_SOCKET;
	}

	boost::system::error_code ec;
	size_t recv_len = receive(asio::buffer(recived), ec);

	if (ec) {
		if (ec == boost::asio::error::operation_aborted) {
			if (g_cfg.log_level > 1) std::cerr << "pc link is not responding.\n";
			return S_COMMUNICATION_ERROR_PC_LINK_TIMEOUT;
		}
		if (ec == boost::asio::error::no_buffer_space) {
			if (g_cfg.log_level > 1) std::cerr << "unusual frame size (>256) recieved.\n";
			return S_COMMUNICATION_ERROR_NO_BUFFER_SIZE;
		}
		// should never happen
		std::cerr << ec.message() << '\n';
		return S_COMMUNICATION_ERROR_SOCKET;
	}

	assert(recv_len && recv_len <= frame::max_size());

	recived.set_length(recv_len);

	if (recv_len == expected_length) {
		if (_timeout_frame == recived) {
			std::cout << "Expected Timeout" << std::endl;
			return 0;
		}
		if (recived.is_crc_valid()) {
			return 0;
		} else {
			if (g_cfg.log_level > 4) std::cerr << "S_ERROR_CRC_RECIVE\n";
			return S_COMMUNICATION_ERROR_CRC_RECIVE;
		}
	} else {
		if (_timeout_frame == recived) {
			if (g_cfg.log_level > 4) std::cerr << "DEVICE DID NOT ANSWER. ADDR: " << (int)output[0] << '\n';
			return S_COMMUNICATION_ERROR_DEVICE_TIMEOUT;
		} else {
			if (g_cfg.log_level > 4) std::cerr << "S_ERROR_UNEXPECTED_FRAMELENGHT" << '\n';
			return S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT;
		}
		return S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT;
	}
}
