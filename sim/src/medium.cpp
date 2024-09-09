// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <sim/medium.h>
#include <algorithm>
#include <chrono>
#include "intrisics.h"

SIM_IMPORT_EXPORT
Medium::Medium()
	: controllers_count_(0) {
	paused_.store(false);
	running_.store(false);
	controllers_.fill(nullptr);
	cpu_frequency_ = (double)GetCpuFrequency(1);
}

void Medium::start() noexcept {
	running_ = true;
	medium_thread_ = std::move(std::thread(&Medium::MediumProc, this));
	core_thread_ = std::move(std::thread(&Medium::CoreProc, this));
}

void Medium::stop() noexcept {
	running_ = false;
	medium_thread_.join();
	core_thread_.join();
}

uint64_t Medium::GetCpuFrequency(uint64_t seconds) {
	volatile int i = 0;
	for (; i < 1000000; ++i);
	auto a = __rdtsc();
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	auto b = __rdtsc();
	return static_cast<uint64_t>((b - a) / seconds);
}

SIM_IMPORT_EXPORT
void Medium::AddController(IController* controller) {
	pause_execution([&]() {
		controllers_[controllers_count_] = controller;
		controllers_count_++;
		controller->SetMedium(this);
	});

	if (running_ == false) start();
}

SIM_IMPORT_EXPORT
bool Medium::RemoveController(IController* controller) {
	auto founded = std::find(controllers_.begin(), controllers_.end(), controller);
	if (founded == controllers_.end()) return false;
	pause_execution([&]() {
		std::copy(std::next(founded), controllers_.end(), founded);
		controllers_count_--;
	});
	
	if (controllers_count_ == 0) stop();

	return true;
}

int Medium::MediumProc() noexcept {
	namespace chrono = std::chrono;
	auto tp2 = chrono::high_resolution_clock::now();
	while (running_.load(std::memory_order_relaxed)) {
		wait_if_paused();
		auto tp1 = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::nanoseconds>(tp1 - tp2);
		tp2 = tp1;

		mstate_.run(duration.count());

		for (size_t ix = 0; ix < controllers_count_; ++ix) {
			controllers_[ix]->ExecutePeripheral(duration.count());
		}
	}
	return 0;
}

int Medium::CoreProc() noexcept {
	try {
		while (running_.load(std::memory_order_relaxed)) {
			wait_if_paused();
			if (!controllers_count_) continue;
			auto ca = __rdtsc();
			auto duration = 0.0;
			for (size_t ix = 0; ix < controllers_count_; ++ix) {
				duration += controllers_[ix]->ExecuteCore();
			}
			duration = duration / (double)controllers_count_;// -0.0000000118;
			uint64_t cc_next = ca + static_cast<uint64_t>(duration * cpu_frequency_);
			if (cc_next < __rdtsc()) {
				// std::cerr << "$\n";
				// runtime_simulation_failed_cnt++;
				continue;
			}
			while (__rdtsc() < cc_next);
		}
	} catch (std::exception& ex) {
		std::cerr << "AVR EXECUTION ERROR: " << ex.what() << '\n';
		return -1;
	}
	return 0;
}