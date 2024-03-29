// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <atomic>

namespace nplib {

class spinlock {
	std::atomic_flag flag_;
public:
	spinlock() : flag_{} {}

	void lock() {
		while (flag_.test_and_set(std::memory_order_acquire));
	}

	void unlock() {
		flag_.clear(std::memory_order_release);
	}
};

} // namespace nplib