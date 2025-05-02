// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <boost/core/exchange.hpp>
#include <cstring>
#include <streambuf>
#include <cstdint>

class membuffer {
	uint8_t* ptr_ = nullptr;
	size_t length_ = 0;
	size_t max_size_ = 0;
public:
	uint8_t* data() noexcept { return ptr_; }
	const uint8_t* data() const noexcept { return ptr_; }
	size_t length() const noexcept { return length_; }

	membuffer() = default;
	
	membuffer(uint8_t* ptr, size_t length, size_t max_size) noexcept
		: ptr_(ptr)
		, length_(length)
		, max_size_(max_size)
	{
	}
	
	membuffer(membuffer&& other) noexcept
		: ptr_(boost::exchange(other.ptr_, nullptr))
		, length_(boost::exchange(other.length_, 0))
		, max_size_(boost::exchange(other.max_size_, 0))
	{			
	}

	~membuffer() {
		if (ptr_) delete[] ptr_;
	}

	membuffer(const membuffer&) = delete;
	membuffer& operator = (const membuffer&) = delete;
};

class omembuf : public std::streambuf {
private:
	char_type* ptr_;
	size_t max_size_;
	size_t initial_size_;
	int64_t saved_size_;
protected:
	virtual int overflow(int c) override {
		max_size_ *= 2;
		char_type* tmp = new char_type[max_size_];

		auto len = length();
		memcpy(tmp, ptr_, len);
		delete[] ptr_;

		ptr_ = tmp;

#if 0 && defined(_MSC_VER)
		setp(ptr_, ptr_ + len, ptr_ + max_size_);
#else
		setp(ptr_, ptr_ + max_size_);
		pbump(static_cast<int>(len));
#endif
		*pptr() = c;
		pbump(1);

		return c;
	}
	virtual pos_type seekoff(off_type offset, 
		std::ios_base::seekdir seek, std::ios_base::openmode /* mode */) {
		saved_size_ = length();
		if (seek == std::ios_base::beg) {
			if (offset < 0 || offset > saved_size_) std::streampos(-1);
#if 0 && defined(_MSC_VER)
			setp(ptr_, ptr_ + offset, ptr_ + max_size_);
#else
			setp(ptr_, ptr_ + max_size_);
			pbump(static_cast<int>(offset));
#endif
			return offset;
		} else if (seek == std::ios_base::end) {
			auto pos = saved_size_ + offset;
			if (offset > 0 || pos < 0) return std::streampos(-1);
#if 0 && defined(_MSC_VER)
			setp(ptr_, ptr_ + pos, ptr_ + max_size_);
#else
			setp(ptr_, ptr_ + max_size_);
			pbump(static_cast<int>(pos));
#endif
			return pos;
		}
		return std::streampos(-1);
	}
public:
	explicit omembuf(size_t initial_size = 512) noexcept 
		: ptr_(nullptr)
		, initial_size_(initial_size)
		, saved_size_(0) {
		clear();
	}
	virtual ~omembuf() {
		if (ptr_) delete[] ptr_;
	}
	
	size_t max_size() const noexcept {
		return max_size_;
	}
	
	size_t length() const noexcept {
		auto len = pptr() - pbase();
		return saved_size_ > len ? saved_size_ : len;
	}
	
	char_type* detach() noexcept {
		char_type* tmp = ptr_;
		ptr_ = nullptr;
		return tmp;
	}

	membuffer transfer_to_membuffer() noexcept {
		return membuffer(reinterpret_cast<uint8_t*>(detach()), length(), max_size());
	}

	const char_type* c_str() const noexcept {
		return ptr_;
	}
	
	void clear() noexcept {
		if (ptr_) delete[] ptr_;
		max_size_ = initial_size_;
		ptr_ = new char_type[initial_size_];
		setp(ptr_, ptr_ + initial_size_);
	}
};
