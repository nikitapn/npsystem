#pragma once

namespace protocol::write {
	class t_write_1 : public t_same_answer
	{
	public:
		t_write_1(uint8_t devAddr, uint16_t address, uint8_t bit, uint8_t value) 
			: t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			frame_s->h.m_addr = 0x00;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = (devAddr == 1) ? F_NOT_TRANSIT | REQ_WRITE_BIT : REQ_WRITE_BIT;
			frame_s->r_write_bit.bit_addr = address;
			frame_s->r_write_bit.bit_n = 1 << bit;
			frame_s->r_write_bit.bit_val = value << bit;
			constexpr auto len = sizeof(net::frame_h) + sizeof(net::req_write_bit);
			frame_s->h.len = static_cast<uint8_t>(len);
			f_->set_length(len);
			f_->write_crc16_le();
		}
	};

	class t_write_1_q : public t_same_answer
	{
	public:
		t_write_1_q(uint8_t devAddr, uint16_t address, uint8_t bit,
			uint8_t value, uint8_t qvalue) : t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			frame_s->h.m_addr = 0x01;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = 0x01;
			frame_s->r_write_bits.nbits = 1;
			frame_s->r_write_bits.data[0].bit_addr = address;
			frame_s->r_write_bits.data[0].bit_mask = ~(3 << bit);
			frame_s->r_write_bits.data[0].bit_val = (value ? (1 << bit) : 0);
			frame_s->r_write_bits.data[0].bit_status = (qvalue ? (2 << bit) : 0);
			constexpr auto len = sizeof(net::frame_h) + sizeof(net::req_write_bits) + sizeof(net::wr_bit_rec) + 2;
			frame_s->h.len = static_cast<uint8_t>(len);
			f_->set_length(len);
			f_->write_crc16_le();
		}
	};

	class t_write_8 : public t_same_answer
	{
	public:
		t_write_8(uint8_t devAddr, uint16_t address,
			uint8_t value, bool withQuality = false, uint8_t qualityValue = 0) 
			: t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			frame_s->h.m_addr = 0x01;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = 0x00;
			frame_s->r_write_bytes.len = 1 + withQuality;
			frame_s->r_write_bytes.addr = address;
			frame_s->r_write_bytes.data[0] = value;
			frame_s->r_write_bytes.data[1] = qualityValue; // status
			const auto len = sizeof(net::frame_h) + sizeof(net::req_write_bytes) + frame_s->r_write_bytes.len + 2;
			frame_s->h.len = static_cast<uint8_t>(len);
			f_->set_length(len);
			f_->write_crc16_le();
		}
		/*
		inline proto_task_write_8()
		{
		std::cout << "proto_task::constructor_8" << std::endl;
		}
		inline proto_task_write_8(const proto_task_write_8& o)
		{
		std::cout << "proto_task::copy_8" << std::endl;
		f_ = o.f_;
		}
		inline proto_task_write_8(proto_task_write_8&& o)
		{
		std::cout << "proto_task::move_8" << std::endl;
		f_ = o.f_;
		o.f_.reset();
		}
		*/
	};

	class t_write_16 : public t_same_answer
	{
	public:
		t_write_16(uint8_t devAddr, uint16_t address,
			uint16_t value, bool withQuality = false, uint8_t qualityValue = 0)
			: t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			frame_s->h.m_addr = 0x01;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = 0x00;
			frame_s->r_write_bytes.len = 2 + withQuality;
			frame_s->r_write_bytes.addr = address;
			*((uint16_t*)frame_s->r_write_bytes.data) = value;
			frame_s->r_write_bytes.data[2] = qualityValue; // status
			const auto len = sizeof(net::frame_h) + sizeof(net::req_write_bytes) + frame_s->r_write_bytes.len + 2;
			frame_s->h.len = static_cast<uint8_t>(len);
			f_->set_length(len);
			f_->write_crc16_le();
		}
	};

	class t_write_32 : public t_same_answer
	{
	public:
		t_write_32(uint8_t devAddr, uint16_t address, 
			uint32_t value, bool withQuality = false, uint8_t qualityValue = 0)
			: t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			frame_s->h.m_addr = 0x01;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = 0x00;
			frame_s->r_write_bytes.len = 4 + withQuality;
			frame_s->r_write_bytes.addr = address;
			*((uint32_t*)frame_s->r_write_bytes.data) = value;
			frame_s->r_write_bytes.data[4] = qualityValue; // status
			const auto len = sizeof(net::frame_h) + sizeof(net::req_write_bytes) + frame_s->r_write_bytes.len + 2;
			frame_s->h.len = static_cast<uint8_t>(len);
			f_->set_length(len);
			f_->write_crc16_le();
		}
	};
	class t_write_block : public t_same_answer
	{
	public:
		t_write_block(uint8_t devAddr, uint16_t address, const void* data, uint8_t length)
			: t_same_answer(devAddr)
		{
			f_ = std::make_shared<frame>();
			net::Frame* frame_s = reinterpret_cast<net::Frame*>(f_.get());

			const auto len = sizeof(net::frame_h) + sizeof(net::req_write_bytes) + length + 2;
			
			frame_s->h.m_addr = 0x00;
			frame_s->h.sl_addr = devAddr;
			frame_s->h.fun_num = (devAddr == 1) ? F_NOT_TRANSIT | REQ_WRITE_BYTES : REQ_WRITE_BYTES;
			frame_s->h.len = static_cast<uint8_t>(len);
			frame_s->r_write_bytes.addr = address;
			frame_s->r_write_bytes.len = length;
			memcpy(frame_s->r_write_bytes.data, data, length);

			f_->set_length(len);
			f_->write_crc16_le();

			// std::cout << *f_ << std::endl;
		}
	};
} // namespace protocol::write
