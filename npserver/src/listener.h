// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <atomic>
#include <vector>
#include <nprpc/flat.hpp>
#include <npc/server.hpp>
#include <npsys/memtypes.h>
#include "environment.h"
#include "protocol.h"
#include "thread_pool.h"
#include <boost/asio/strand.hpp>

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
	boost::asio::io_context::strand strand_;

	void advise(const nprpc::flat::Span<const nps::flat::DataDef>& a, nprpc::flat::Span<nps::var_handle>& handles);
	void release_one(protocol::client_handle handle);
	void release_all();
public:
	virtual void OnDataChanged(const std::vector<nps::server_value>& ar) = 0;

	auto t_process_registers(const std::shared_ptr<updated_registers_t>& registers) {
		task_count_++;
		return nplib::async<false>(strand_, &listener::process_registers, this, registers);
	}

	// wait only function
	[[nodiscard]]
	auto t_advise(const nprpc::flat::Span<const nps::flat::DataDef>& a, nprpc::flat::Span<nps::var_handle>& handles) {
		task_count_++;
		return nplib::async<true>(strand_, &listener::advise, this, std::cref(a), std::ref(handles)).get();
	}

	auto t_release_all() {
		task_count_++;
		return nplib::async<false>(strand_, &listener::release_all, this);
	}

	auto task_count() const noexcept { return task_count_.load(std::memory_order_relaxed); }
	
	listener() : strand_(thread_pool::get_instance().ctx()) {}
};

}