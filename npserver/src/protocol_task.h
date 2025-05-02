// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "environment.h"

namespace protocol {

class t_one_time {
	friend class protocol_service;
public:
	t_one_time(int dev_addr) : dev_addr_(dev_addr) {}
protected:
	std::shared_ptr<frame> f_;
	int dev_addr_;
};

class t_same_answer : public t_one_time {
public:
	t_same_answer(int dev_addr) : t_one_time(dev_addr) {}
	int operator() () 
	{
		frame answer;
		int result = environment::get_instance().get_bridge(dev_addr_)->send_recive(*f_, answer, f_->length());
		if (result < 0) {
			return result;
		} else if ((result >= 0) && (*f_ == answer)) {
			return 0;
		} else {
			return S_COMMUNICATION_ERROR_UNEXPECTED_FRAME;
		}
	}
};

class t_no_answer : public t_one_time {
public:
	t_no_answer(int dev_addr) : t_one_time(dev_addr) {}
	int operator() () 
	{
		frame answer;
		return environment::get_instance().get_bridge(dev_addr_)->send_recive(*f_, answer, 4);
	}
};

class t_send_raw : public t_no_answer {
public:
	t_send_raw(const void* data, const uint8_t len)
		: t_no_answer(1)
	{
		f_ = std::make_shared<frame>();
		std::memcpy(f_->data(), data, len);
		f_->set_length(len);
	}
};
} // namespace protocol
