// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "protocol.h"
#include "config.h"
#include "environment.h"
#include "history.h"
#include "itemmgr.h"
#include "listener.h"
#include <nplib/utils/binary_search.hpp>

namespace protocol {

boost::pool<> Register::pool_(sizeof(Register), 256);

protocol_service::protocol_service() {
	memset(dev_reqs_, 0, sizeof(DeviceRequest) * MAX_CTRLS);

	for (size_t i = 0; i < MAX_CTRLS; i++) {
		dev_reqs_[i].dev_addr = i;
	}

	worker_ = std::move(std::thread(&protocol_service::worker, this));
}

void protocol_service::worker() {
	using Result = nplib::task_queue::Result;
	if (g_cfg.log_level > 1) {
		std::cout << "protocol_service: worker started." << std::endl;
	}
	std::chrono::steady_clock::time_point deadline;
	int socket_error_timeout = 0;
	while (true) {
		auto const res = tasks_.process_task_until(deadline);
		if (res == Result::Exit) break;

		if (res == Result::Timeout) {
			if (read_advised() == S_COMMUNICATION_ERROR_SOCKET) {
				if (socket_error_timeout < 1000) {
					socket_error_timeout += 250;
				} else if (socket_error_timeout < 5000) {
					socket_error_timeout += 1000;
				} else {
					// socket_error_timeout == 5000 max
				}
			} else {
				socket_error_timeout = 0;
			}
		}
		deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(g_cfg.read_period_ms + socket_error_timeout);
	}
	if (g_cfg.log_level > 1) {
		std::cout << "protocol_service: worker terminated." << std::endl;
	}
}

int protocol_service::add_one_register(NewRegister r) {
	if (protocol_service::add_register(r) == 0) {
		if (g_cfg.log_level > 2) print_table();
		return 0;
	}
	rebuild();
	return 1;
}

int protocol_service::add_registers(newregs_t& regs) noexcept {
	int added = 0;
	for (auto& r : regs) {
		added += add_register(r);
	}

	if (!added) {
		if (g_cfg.log_level > 2) print_table();
		return 0;
	}

	rebuild();
	return added;
}

int protocol_service::release_one_register(released_reg_t reg) noexcept {
	released_regs_t tmp = { reg };
	return release_registers(tmp);
}

int protocol_service::release_registers(released_regs_t& regs) noexcept {
	bool del = false;
	auto vc_end = m_valid_controllers.end();

	for (auto& r : regs) {
		if (m_valid_controllers.find(r.first.dev_addr) == vc_end) {
			std::cout << "device deleted" << std::endl;
			continue;
		}
		wrarray_t::iterator founded;
		wrarray_t::iterator begin = dev_reqs_[r.first.dev_addr].r.a.begin();
		wrarray_t::iterator end = begin + dev_reqs_[r.first.dev_addr].r.n;
		if (nplib::binary_find(begin, end, r.first, founded, 
			[](const Register_Ptr pReg, const server_handle& val) { return pReg->handle.value < val.value; }, 
			[](const Register_Ptr pReg, const server_handle& val) { return pReg->handle.value == val.value; })
			)
		{
			(*founded)->ref_cnt -= r.second;
			del |= ((*founded)->ref_cnt <= 0 ? true : false);
		} else {
			std::cerr << "item handle: " << std::hex << r.first.value << " wasn't founded\n";
		}
		m_tag_changed.insert(r.first.dev_addr);
	}

	if (!del) return 0;

	rebuild();
	return 1;
}

void protocol_service::rebuild() {
	for (auto i : m_tag_changed)
		build_req(dev_reqs_[i]);

	m_tag_changed.clear();
	if (g_cfg.log_level > 2) print_table();

	advc_it_ = m_valid_controllers.begin();
	req_ix_ = 0;
}

int protocol_service::remove_device(int dev_addr) noexcept {
	m_valid_controllers.erase(dev_addr);
	advc_it_ = m_valid_controllers.begin();
	req_ix_ = 0;

	auto& ctrl = dev_reqs_[dev_addr];

	for (size_t i = 0; i < ctrl.r.n; i++) {
		delete ctrl.r.a[i]; 
	}

	ctrl.r.n = 0;
	ctrl.q.n = 0;

	return 0;
}

void protocol_service::print_table() {
	std::cout << "REQUESTS TABLE:\n";
	for (auto i : m_valid_controllers) {
		const auto& req = dev_reqs_[i];
		std::cout << "  DEVICE: " << (int)i << "\n";
		for (size_t i = 0; i < req.q.n; i++) {
			std::cout << "      REQUEST: " << i << "\tREQ_SZ: " << (int)req.q.a[i].nBytes << "\n";
			for (uint8_t r = 0; r < req.q.a[i].nRegister; r++) {
				std::cout << "        Address: " << std::hex << (int)req.q.a[i].data[r]->handle.mem_addr <<
					"\tSize: " << std::dec << (int)req.q.a[i].data[r]->handle.size <<
					"\tRefCnt: " << (int)req.q.a[i].data[r]->ref_cnt << "\n";
			}
		}
	}
	std::cout.flush();
}

// ������� ��������� ����� ������� � ������ ��������������� ���������
int protocol_service::add_register(NewRegister& r) {
	auto handle = (server_handle)r.c_handle;
	assert(handle.size > 0 && handle.size <= 5);
	auto& req = dev_reqs_[handle.dev_addr];

	auto const begin = req.r.a.begin();
	auto const end = begin + req.r.n;

	wrarray_t::iterator it;

	if (nplib::binary_find(begin, end, handle, it,
		[](const Register_Ptr pReg, const server_handle& val) {
			return pReg->handle.value < val.value; },
		[](const Register_Ptr pReg, const server_handle& val) {
				return pReg->handle.value == val.value; })) {
		r.ptr = *it;
		(*it)->ref_cnt++;
		return 0;
	}

	// std::cout << (int)(it - begin) << endl;


	if (it != end) {
		std::move(it, end, std::next(it));
		//		std::cout << "moved: " << (int)(it - begin) << endl;
	}

	r.ptr = (*it) = new Register(handle);

	req.r.n++;

	m_tag_changed.insert(handle.dev_addr);

	return 1;
}

void protocol_service::build_req(DeviceRequest& req) {
	wrarray_t tmp;
	size_t k = 0;

	for (size_t i = 0; i < req.r.n; i++) {
		if ((req.r.a[i]->ref_cnt <= 0)) {
			delete req.r.a[i];
		} else {
			tmp[k++] = req.r.a[i];
		}
	}

	if (k == 0) {
		m_valid_controllers.erase(req.dev_addr);
		req.r.n = 0;
		req.q.n = 0;
		return;
	}

	m_valid_controllers.insert(req.dev_addr);

	if (req.r.n != k) {
		auto first = tmp.begin();
		auto last = first;
		std::advance(last, k);
		std::copy(first, last, req.r.a.begin());
		req.r.n = k;
	}

	size_t q_count = 0;
	for (size_t r = 0; r < req.r.n; r++) {
		uint8_t nReqRegCount = 0;
		req.q.a[q_count].data[nReqRegCount] = req.r.a[r];
		uint8_t ReqSize = req.r.a[r]->handle.size;
		nReqRegCount++;
		for (size_t i = r + 1; i < req.r.n && nReqRegCount < MAX_REG_IN_REQ; ++i) {
			if (
				req.r.a[i - 1]->handle.mem_addr + req.r.a[i - 1]->handle.size ==
				req.r.a[i]->handle.mem_addr
				) {
				req.q.a[q_count].data[nReqRegCount] = req.r.a[i];
				ReqSize += req.r.a[i]->handle.size;
				nReqRegCount++;
				r++;
			} else {
				break;
			}
		}
		req.q.a[q_count].nBytes = ReqSize;
		req.q.a[q_count].nRegister = nReqRegCount;
		q_count++;
	}
	req.q.n = q_count;
}

int protocol_service::add_listener(listener* ptr) noexcept {
	listeners_.push_back(ptr);
	return 0;
}

int protocol_service::remove_listener(listener* ptr) noexcept {
	if (auto it = std::find(listeners_.begin(), listeners_.end(), ptr); it != listeners_.end()) {
		listeners_.erase(it);
		if (g_cfg.log_level > 1) {
			std::cout << "listeners_.size: " << listeners_.size() << std::endl;
		}
		return 0;
	} else {
		std::cerr << "Error: listener not found\n";
	}
	return -1;
}

int protocol_service::read_advised() noexcept {
	if (m_valid_controllers.empty()) return 0;

	auto updated_registers = std::make_shared<updated_registers_t>();

	auto* frame_s = reinterpret_cast<net::Frame*>(&buf_out_);
	auto* frame_r = reinterpret_cast<net::Frame*>(&buf_in_);

	frame_s->h.m_addr = 0x01;

	uint8_t c = *advc_it_;
	size_t i = req_ix_;

	const uint8_t& nBytes = dev_reqs_[c].q.a[i].nBytes;

	frame_s->h.sl_addr = c;
	frame_s->h.fun_num = (c == 1) ? 0x80 | 0x02 : 0x02;
	frame_s->r_read_bytes.addr_begin = dev_reqs_[c].q.a[i].data[0]->handle.mem_addr;
	frame_s->r_read_bytes.nbytes = nBytes;
	constexpr uint8_t len = sizeof(net::frame_h) + sizeof(net::req_read_bytes);
	frame_s->h.len = len;
	buf_out_.set_length(len);
	buf_out_.write_crc16_le();

	int result = environment::get_instance().
		get_bridge(c)->send_recive(buf_out_, buf_in_, 
			sizeof(net::frame_h) + sizeof(net::ans_read_bytes) + nBytes + 2);

	if (result < 0) {
		for (int r = 0; r < dev_reqs_[c].q.a[i].nRegister; r++) {
			Register_Ptr pReg = dev_reqs_[c].q.a[i].data[r];
			if (pReg->op_available != RegStatus::REG_DEVICE_NOT_RESPONDED) {
				pReg->op_available = RegStatus::REG_DEVICE_NOT_RESPONDED;
				updated_registers->push_back(pReg);
			}
		}
	} else {
		uint8_t* p = frame_r->a_read_bytes.data;
		for (int r = 0; r < dev_reqs_[c].q.a[i].nRegister; r++) {
			Register_Ptr pReg = dev_reqs_[c].q.a[i].data[r];

			uint64_t changed = (pReg->op_available != RegStatus::REG_AVAILABLE);
			Value val;

			switch (pReg->handle.size)
			{
			case 1: // simple byte no status
				val.u8 = *reinterpret_cast<const uint8_t*>(p);
				changed |= pReg->op_value.u8 ^ val.u8;
				pReg->op_value.u8 = val.u8;
				break;
			case 2: // simple word no status or byte with status
				val.u16 = *reinterpret_cast<const uint16_t*>(p);
				changed |= pReg->op_value.u16 ^ val.u16;
				pReg->op_value.u16 = val.u16;
				break;
			case 3: // word with status
				val.u32 = *reinterpret_cast<const uint32_t*>(p) & 0xFFFFFF;
				changed |= pReg->op_value.u32 ^ val.u32;
				pReg->op_value.u32 = val.u32;
				break;
			case 4: // simple dword no status
			//	std::cout << "dword" << std::endl;
				val.u32 = *reinterpret_cast<const uint32_t*>(p);
				changed |= pReg->op_value.u32 ^ val.u32;
				pReg->op_value.u32 = val.u32;
				break;
			case 5: // dword with status
			//	std::cout << "dword + status" << std::endl;
				val.u64 = *reinterpret_cast<const uint64_t*>(p) & 0xFF'FFFF'FFFF;
				changed |= pReg->op_value.u64 ^ val.u64;
				pReg->op_value.u64 = val.u64;
				break;
			default:
				assert(false);
				break;
			}
			p += pReg->handle.size;
			if (changed) {
				pReg->op_available = RegStatus::REG_AVAILABLE;
				updated_registers->push_back(pReg);
			}
		}
	}

	if (++req_ix_ >= dev_reqs_[*advc_it_].q.n) {
		req_ix_ = 0;
		if (++advc_it_ == m_valid_controllers.end()) {
			advc_it_ = m_valid_controllers.begin();
		}
	}

	if (!updated_registers->empty()) {
		for (auto ptr : listeners_) {
			ptr->t_process_registers(updated_registers);
		}
//		hist->t_process_registers(updated_registers);
	}

	return result;
}

} // namespace protocol