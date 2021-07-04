// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <ostream>
#include <iomanip>
#include <algorithm>

class frame_invalid_size : public std::runtime_error {
public:
	frame_invalid_size() : std::runtime_error("invalid frame size") {}
};

class frame {
	static constexpr auto max_size_ = 256; // don't change
	
	using buffer_t = std::array<uint8_t, max_size_>;
	buffer_t data_;
	size_t length_;

	uint16_t get_crc16() const noexcept;
	bool is_size_valid() const noexcept {
		return length_ >= 4;
	}
public:
	static constexpr auto max_size() noexcept {
		return max_size_;
	}

	frame(const frame& other) = delete;
	frame(frame&& other) = delete;
	frame& operator= (const frame& other) = delete;
	frame& operator= (frame&& other) = delete;

	frame() noexcept : length_(0) {};
	frame(std::initializer_list<uint8_t> lst) noexcept {
		set_length(lst.size());
		std::copy(lst.begin(), lst.end(), data_.begin());
	}

	void set_length(size_t length) noexcept {
		assert(length <= max_size_);
		length_ = length;
	}
	
	size_t length() const noexcept {
		return length_;
	}
	
	bool is_crc_valid() const noexcept {
		if (!is_size_valid()) return false;
		return (get_crc16() == *reinterpret_cast<const uint16_t*>(&data_[length_ - 2]));
	}
	
	uint8_t& operator[](size_t ix) {
		return data_[ix];
	}
	
	void write_crc16_le() noexcept {
		if (!is_size_valid()) {
			assert(false); 
			return;
		}
		
		uint16_t crc = get_crc16();
		data_[length_ - 2] = crc & 0xff;
		data_[length_ - 1] = (crc >> 8) & 0xff;
	}

	operator const uint8_t*() const {
		return &data_[0];
	}

	uint8_t* data() noexcept {
		return &data_[0];
	}

	const uint8_t* data() const noexcept {
		return &data_[0];
	}

	bool operator == (const frame& other) const noexcept {
		if (length_ != other.length_) return false;
		for (size_t i = 0; i < length_; ++i) {
			if (data_[i] != other.data_[i]) return false;
		}
		return true;
	}

	bool operator != (const frame& other) const noexcept {
		return !this->operator==(other);
	}
};

namespace boost::asio {
inline const_buffers_1 buffer(const frame& f) {
	return boost::asio::buffer(f.data(), f.length());
}

inline mutable_buffers_1 buffer(frame& f) {
	return boost::asio::buffer(f.data(), frame::max_size());
}
} // namespace boost::asio

inline std::ostream& operator<<(std::ostream& os, const frame& f) noexcept {
	os << std::hex << std::setfill('0');
	for (size_t i = 0; i < f.length(); ++i) {
		os << std::setw(2) << (int)f[i] << ' ';
 	}
	os << std::dec << std::setfill(' ');
	return os;
}

inline static frame _timeout_frame = { 0xff, 'T', 0x41, 0xbf };