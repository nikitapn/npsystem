#pragma once

#include <atomic>
#include <vector>
#include <npc/server.hpp>
#include <npsys/memtypes.h>
#include "environment.h"
#include "protocol.h"
#include "thread_pool.h"
#include <boost/asio/strand.hpp>

template<typename T>
struct Span {
	T* begin_;
	T* end_;
	
	const T* begin() const noexcept { return begin_; }
	const T* end() const noexcept { return end_; }
	T* begin() noexcept { return begin_; }
	T* end() noexcept { return end_; }
	size_t size() const noexcept { return end_ - begin_; }
};

namespace protocol {
using updated_registers_t = std::vector<protocol::Register const*>;

class listener {
	std::unordered_map<
		protocol::Register const*,
		std::vector<std::pair<protocol::client_handle, uint32_t> >
	> hmap_;

	uint32_t last_id_ = 1;
	std::atomic_uint64_t task_count_ = 0; // temp

	void process_registers(std::shared_ptr<updated_registers_t> registers); // !!!! only by value
protected:
	void advise(const Span<nps::flat::DataDef>& a, Span<nps::var_handle>& handles);
	void release_one(protocol::client_handle handle);
	void release_all();
public:
	virtual void OnDataChanged(const std::vector<nps::server_value>& ar) = 0;

	auto t_process_registers(const std::shared_ptr<updated_registers_t>& registers) {
		task_count_++;
		return mylib::async<false>(strand_, &listener::process_registers, this, registers);
	}

	// wait only function
	[[nodiscard]]
	auto t_advise(const Span<nps::flat::DataDef>& a, Span<nps::var_handle>& handles) {
		task_count_++;
		return mylib::async<true>(strand_, &listener::advise, this, std::cref(a), std::ref(handles)).get();
	}

	auto t_release_all() {
		task_count_++;
		return mylib::async<false>(strand_, &listener::release_all, this);
	}

	auto task_count() const noexcept { return task_count_.load(std::memory_order_relaxed); }
	
	listener() : strand_(thread_pool::get_instance().ctx()) {}
protected:
	boost::asio::io_context::strand strand_;
};

}