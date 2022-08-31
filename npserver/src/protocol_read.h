// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nprpc/flat.hpp>

namespace protocol::read {
class t_read_byte : public t_one_time {
public:
	t_read_byte(uint8_t devAddr, uint16_t mem_addr, uint8_t& value) :
		t_one_time(devAddr), value_(value)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr uint8_t len = sizeof(net::frame_h) + sizeof(net::req_read_bytes);

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = devAddr;
		frame_s->h.fun_num = (devAddr == 1) ? F_NOT_TRANSIT | 0x02 : 0x02;
		frame_s->h.len = len;
		frame_s->r_read_bytes.addr_begin = mem_addr;
		frame_s->r_read_bytes.nbytes = 1;
		f_->set_length(len);
		f_->write_crc16_le();
	}
	int operator() () {
		frame answer;
		int result = environment::get_instance().get_bridge(dev_addr_)->send_recive(*f_, answer, sizeof(net::frame_h) + sizeof(net::ans_read_bytes) + 1 + 2);
		if (result >= 0) {
			net::Frame* frame_r = reinterpret_cast<net::Frame*>(&answer[0]);
			value_ = frame_r->a_read_bytes.data[0];
		}
		return result;
	}
private:
	uint8_t& value_;
};

class t_read_memory : public t_one_time {
	uint8_t len_to_read_;
	nprpc::flat::Vector_Direct1<uint8_t>& vd_;
public:
	t_read_memory(uint8_t devAddr, uint16_t mem_addr, uint8_t len_to_read, nprpc::flat::Vector_Direct1<uint8_t>& vd) :
		t_one_time(devAddr), len_to_read_(len_to_read), vd_(vd)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr uint8_t len = sizeof(net::frame_h) + sizeof(net::req_read_bytes);

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = devAddr;
		frame_s->h.fun_num = (devAddr == 1) ? F_NOT_TRANSIT | 0x02 : 0x02;
		frame_s->h.len = len;
		frame_s->r_read_bytes.addr_begin = mem_addr;
		frame_s->r_read_bytes.nbytes = len_to_read;
		f_->set_length(len);
		f_->write_crc16_le();
	}

	int operator() () {
		frame answer;
		int result = environment::get_instance().get_bridge(dev_addr_)->send_recive(*f_, answer,
			sizeof(net::frame_h) + sizeof(net::ans_read_bytes) + len_to_read_ + 2);
		if (result >= 0) {
			net::Frame* frame_r = reinterpret_cast<net::Frame*>(&answer[0]);
			vd_.length(len_to_read_);
			std::memcpy(vd_().data(), frame_r->a_read_bytes.data, len_to_read_);
		}
		return result;
	}
};

} // namespace protocol::read
