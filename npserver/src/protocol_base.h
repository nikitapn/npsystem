// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "frame.h"
#include "server.h"
#include "bridge.h"
#include "regstatus.h"
#include <avr_firmware/net.h>
#include <functional>
#include <nplib/utils/task_queue.h>

namespace protocol {

struct Value {
	union {
		struct {
			uint8_t value;
			uint8_t quality;
		} qu8;
		struct {
			uint16_t value;
			uint8_t quality;
		} qu16;
		struct {
			uint32_t value;
			uint8_t quality;
		} qu32;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		uint8_t data[8];
	};
};

class server_handle;

class client_handle {
public:
	union {
		uint64_t handle;
		struct {
			union {
				uint32_t value;
				struct {
					uint8_t dev_addr;
					uint8_t size;
					uint16_t mem_addr; // should be the last for correct binary search
				};
			};
			uint32_t id;
		};
	};

	client_handle() = default;
	client_handle(uint64_t _handle)
		: handle(_handle) {}
	
	client_handle(uint8_t _dev, uint16_t _mem, uint8_t _size, uint32_t id) noexcept {
		this->dev_addr = _dev;
		this->size = _size;
		this->mem_addr = _mem;
		this->id = id;
	}

	client_handle(const server_handle&) = delete;
	client_handle(server_handle&&) = delete;
	client_handle& operator=(const server_handle&) = delete;
	client_handle& operator=(server_handle&&) = delete;

	bool operator == (const client_handle& other) const noexcept {	
		return handle == other.handle;
	}

	bool operator < (const client_handle& other) const noexcept {
		return handle < other.handle;
	}
};

class server_handle : public client_handle {
public:
	server_handle() = default;
	
	server_handle(const client_handle& other) {
		this->value = other.value;
		this->id = 0;
	}

	server_handle& operator=(const client_handle& other) {
		this->value = other.value;
		this->id = 0;
		return (*this);
	}
	
	server_handle(client_handle&&) = delete;
	server_handle& operator=(client_handle&&) = delete;
};

class Register {
	static boost::pool<> pool_;
public:
	server_handle handle;
	Value op_value;
	
	RegStatus op_available;
	int ref_cnt;
	
	explicit inline Register(server_handle handle) {
		this->handle = handle;
		this->ref_cnt = 1;
		this->op_available = RegStatus::REG_UNKNOWN;
	}
	
	static void* operator new(size_t n) {
		//static auto t_id = std::this_thread::get_id();
		//assert(t_id == std::this_thread::get_id());
		return pool_.malloc();
	}
	
	static void operator delete(void* ptr) {
		//static auto t_id = std::this_thread::get_id();
		//assert(t_id == std::this_thread::get_id());
		pool_.free(ptr);
	}
};

using Register_Ptr = Register*;

struct Request {
	uint8_t nRegister;
	uint8_t nBytes;
	std::array<Register_Ptr, MAX_REG_IN_REQ> data;
};

using wrarray_t = std::array<Register_Ptr, 1024>;
using rqarray_t = std::array<Request, 128>;

struct NewRegister {
	NewRegister(client_handle c, Register const * p) 
		: c_handle(c), ptr(p) {}

	const client_handle c_handle;
	Register const * ptr;
};

using released_reg_t = std::pair<server_handle, short>;
using newregs_t = std::vector<NewRegister>;
using released_regs_t = std::vector<released_reg_t>;

class listener;

class protocol_service 
	: public boost::noncopyable 
{
	struct DeviceRequest {
		uint8_t dev_addr;
		// all sorted registers
		struct {
			size_t n;
			wrarray_t a;
		} r;
		// requests
		struct {
			size_t n;
			rqarray_t a;
		} q;
	};

	nplib::task_queue tasks_;
	std::thread worker_;
	std::set<uint8_t> m_tag_changed;
	std::set<uint8_t> m_valid_controllers;
	std::set<uint8_t>::iterator advc_it_;
	size_t req_ix_;
	DeviceRequest dev_reqs_[MAX_CTRLS];
	frame buf_out_;
	frame buf_in_;
	std::vector<listener*> listeners_;

	//
	void worker();
	int read_advised() noexcept;
	void rebuild();
	void build_req(DeviceRequest& req);
	void print_table();
	int add_register(NewRegister& r);
	int add_one_register(NewRegister r);
	int add_registers(newregs_t& regs) noexcept;
	int release_one_register(released_reg_t reg) noexcept;
	int release_registers(released_regs_t& regs) noexcept;
	int remove_device(int dev_addr) noexcept;

	int add_listener(listener* ptr) noexcept;
	int remove_listener(listener* ptr) noexcept;
public:
	void stop() noexcept {
		tasks_.exit();
		worker_.join();
	}
	template<typename F>
	std::future<int> t_add_task(F&& obj) noexcept {
		return tasks_.add_task(std::forward<F>(obj));
	}

	std::future<int> t_add_one_register(NewRegister reg) noexcept {
		return tasks_.add_task(std::bind(&protocol_service::add_one_register, this, reg));
	}

	// wait only function
	[[nodiscard]]
	std::future<int> t_add_registers(newregs_t& regs) noexcept {
		return tasks_.add_task(std::bind(&protocol_service::add_registers, this, std::ref(regs)));
	}

	std::future<int> t_release_registers(released_regs_t&& regs) noexcept {
		return tasks_.add_task(
			[rr{ std::move(regs) }, this]() mutable { return release_registers(rr); });
			//std::bind(&protocol_service::release_registers, this, regs));
	}

	std::future<int> t_remove_device(int dev_addr) noexcept {
		return tasks_.add_task(std::bind(&protocol_service::remove_device, this, dev_addr));
	}

	std::future<int> t_add_listener(listener* ptr) noexcept {
		return tasks_.add_task(std::bind(&protocol_service::add_listener, this, ptr));
	}

	std::future<int> t_remove_listener(listener* ptr) noexcept {
		return tasks_.add_task(std::bind(&protocol_service::remove_listener, this, ptr));
	}

	protocol_service();
};


} // namespace protocol

#include <boost/functional/hash.hpp>

namespace std {
template<>
struct hash<protocol::server_handle> {
	size_t operator()(const protocol::server_handle& handle) const noexcept {
		std::size_t val = 0;
		boost::hash_combine(val, handle.dev_addr);
		boost::hash_combine(val, handle.mem_addr);
		boost::hash_combine(val, handle.size);
		return val;
	}
};

}
