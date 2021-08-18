// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "listener.h"
#include <npsys/variable.h>
#include <npc/server.hpp>
#include <nplib/utils/binary_search.hpp>

namespace protocol {
// strand context
void listener::advise(const Span<nps::flat::DataDef>& a, Span<nps::var_handle>& handles) {
	assert(a.size() == handles.size());

	for (auto const& i: a) {
		environment::get_instance().validate_device(i.dev_addr);
	}

	// std::cout << "Advise" << std::endl;
	protocol::newregs_t registers;
	auto it = handles.begin();

	for (auto const& i: a) {
		const int type = i.type;

		client_handle c_handle(
			i.dev_addr,
			i.mem_addr,
			static_cast<uint8_t>(npsys::variable::GetSize(type) + (npsys::variable::IsBit(type) ? 0 : npsys::variable::IsQuality(type))), 
			last_id_++);

		(*it) = c_handle.handle;
		it++;

		registers.emplace_back(c_handle, nullptr);
	}

	proto->t_add_registers(registers).get();

	auto send_registers_now = std::make_shared<updated_registers_t>();

	size_t ix = 0;
	for (auto const& i: a) {
		auto& r = registers[ix++];
		hmap_[r.ptr].push_back({ r.c_handle, i.type });
		if (r.ptr->op_available != RegStatus::REG_UNKNOWN) send_registers_now->push_back(r.ptr);
	}

	if (!send_registers_now->empty()) {
		t_process_registers(send_registers_now);
	}

	task_count_--;
}

// strand context
void listener::release_one(protocol::client_handle c_handle) {
	if (auto it = std::find_if(hmap_.begin(), hmap_.end(), 
			[=](const auto& x) { return x.second[0].first.value == c_handle.value; }); 
		it != hmap_.end()) 
	{
		if (it->second.size() == 1) {
			hmap_.erase(it);
		} else {
			if (auto it_handle = std::find_if(it->second.begin(), it->second.end(),
				[=](const auto& x) { return x.first == c_handle; }); 
				it_handle != it->second.end()) 
			{
				it->second.erase(it_handle);
			}
		}
	}
	proto->t_release_registers(released_regs_t{ std::make_pair(server_handle(c_handle), 1) });
}

// strand context
void listener::process_registers(std::shared_ptr<updated_registers_t> registers) {
	std::vector<nps::server_value> ar;

	auto end = hmap_.end();

	for (auto reg : *registers) {
		auto it = hmap_.find(reg);
		if (it == end) continue;
	
		for (const auto& ch : it->second) {
			ar.push_back({});
			auto& back = ar.back();
			
			back.h = ch.first.handle;
			auto type = ch.second;

			//	cout << "type: " << std::hex << (int)type << endl;

			if (reg->op_available == RegStatus::REG_DEVICE_NOT_RESPONDED) {
				back.s = nps::var_status::VST_DEVICE_NOT_RESPONDED;
				continue;
			}

			back.s = nps::var_status::VST_DEVICE_RESPONDED;

			if (npsys::variable::IsBit(type)) {
				int bit_n = npsys::variable::GetBit(type);
				back.data[0] = ((1 << bit_n) & reg->op_value.u8) ? NPSYSTEM_TRUE : NPSYSTEM_FALSE;
				if (npsys::variable::IsQuality(type)) {
					back.data[1] = ((1 << (bit_n + 1)) & reg->op_value.u8) ? VQ_GOOD : VQ_BAD;
				}
			} else {
				*reinterpret_cast<uint64_t*>(&back.data[0]) = reg->op_value.u64;
			}
		}
	}

	//	cout << "Updated: " << upd_size << endl;

	if (!ar.empty()) OnDataChanged(ar);
	task_count_--;
}

// strand context
void listener::release_all() {
	released_regs_t regs;
	for (auto& x : hmap_) {
		regs.emplace_back(server_handle(x.second[0].first), x.second.size());
	}
	hmap_.clear();
	proto->t_release_registers(std::move(regs));
}

} // namespace protocol