// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

namespace protocol::avr5 {
class t_stop_algorithm : public t_one_time {
public:
	t_stop_algorithm(uint8_t dev_addr, uint16_t alg_addr)
		: t_one_time(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr auto len = sizeof(net::frame_h) + sizeof(net::req_stop_alg);

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = ((dev_addr == 1)
			? F_NOT_TRANSIT | REQ_STOP_ALG
			: REQ_STOP_ALG);
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_stop_alg.addr = alg_addr;

		f_->set_length(len);
		f_->write_crc16_le();
	}
	int operator() ()
	{
		frame answer;
		int result = environment::get_instance().get_bridge(dev_addr_)->send_recive(*f_, answer, sizeof(net::frame_h) + sizeof(net::ans_stop_alg));
		if (result >= 0) {
			net::Frame* frame_r = reinterpret_cast<net::Frame*>(&answer[0]);
			return frame_r->a_stop_alg.result;
		}
		return result;
	}
};

class t_reinit_io : public t_same_answer {
public:
	t_reinit_io(uint8_t dev_addr)
		: t_same_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr auto len = sizeof(net::frame_h) + 2;

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = REQ_REINIT_IO;
		frame_s->h.len = static_cast<uint8_t>(len);

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_update_remote_data : public t_no_answer {
public:
	t_update_remote_data(uint8_t dev_addr, uint16_t page_num, const void* data, const size_t length)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		auto len = sizeof(net::frame_h) + sizeof(net::req_write_rd) + length + 2;

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_WRITE_RD :
			F_NO_ANSWER | REQ_WRITE_RD;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_write_rd.page = static_cast<uint8_t>(page_num); //!
		frame_s->r_write_rd.size = static_cast<uint8_t>(length);

		memcpy(frame_s->r_write_rd.data, data, length);

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_send_page : public t_no_answer {
public:
	t_send_page(uint8_t dev_addr, uint16_t page_num, const void* data, const size_t length)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		auto len = sizeof(net::frame_h) + sizeof(net::req_write_page) + length + 2;

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_WRITE_PAGE :
			F_NO_ANSWER | REQ_WRITE_PAGE;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_wr_p.page = static_cast<uint8_t>(page_num);
		frame_s->r_wr_p.size = static_cast<uint8_t>(length);

		memcpy(frame_s->r_wr_p.data, data, length);

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_remove_alg : public t_no_answer {
public:
	t_remove_alg(uint8_t dev_addr, uint16_t alg_addr)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr auto len = sizeof(net::frame_h) + sizeof(net::req_remove_alg);

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_REMOVE_ALG :
			F_NO_ANSWER | REQ_REMOVE_ALG;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_remove_alg.addr = alg_addr;

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_replace_alg : public t_no_answer {
public:
	t_replace_alg(uint8_t dev_addr, uint16_t old_addr, uint16_t new_addr)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		constexpr auto len = sizeof(net::frame_h) + sizeof(net::req_replace_alg);

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_REPLACE_ALG :
			F_NO_ANSWER | REQ_REPLACE_ALG;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_rep_alg.addr_old = old_addr;
		frame_s->r_rep_alg.addr_new = new_addr;

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_write_eeprom : public t_no_answer {
public:
	t_write_eeprom(uint8_t dev_addr, uint16_t mem_addr, const void* data, const size_t length)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		const auto len = sizeof(net::frame_h) + sizeof(net::req_write_epprom) + length + 2;

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_WRITE_EEPROM :
			F_NO_ANSWER | REQ_WRITE_EEPROM;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_write_epprom.addr = mem_addr;
		frame_s->r_write_epprom.len = static_cast<uint8_t>(length);
		memcpy(frame_s->r_write_epprom.data, data, length);

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

class t_write_twi_table : public t_no_answer {
public:
	t_write_twi_table(uint8_t dev_addr, uint16_t page_num, const void* data, const size_t length)
		: t_no_answer(dev_addr)
	{
		f_ = std::make_shared<frame>();
		net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

		const auto len = sizeof(net::frame_h) + sizeof(net::req_write_twi) + length + 2;

		frame_s->h.m_addr = 0x00;
		frame_s->h.sl_addr = dev_addr;
		frame_s->h.fun_num = (dev_addr == 1) ?
			F_NOT_TRANSIT | F_NO_ANSWER | REQ_WRITE_TWI_TAB :
			F_NO_ANSWER | REQ_WRITE_TWI_TAB;
		frame_s->h.len = static_cast<uint8_t>(len);
		frame_s->r_write_twi.page = static_cast<uint8_t>(page_num);
		frame_s->r_write_twi.size = static_cast<uint8_t>(length);
		memcpy(frame_s->r_write_twi.data, data, length);

		f_->set_length(len);
		f_->write_crc16_le();
	}
};

} // namespace protocol::avr5
