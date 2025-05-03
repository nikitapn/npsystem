// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <stdint.h>
#include <array>
#include <sim/import_export.h>
#include "special.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <boost/serialization/singleton.hpp>


class Medium;

class IController {
public:
	/**
	   * Returns the time in nano seconds spent executing one single instruction.
	   */
	virtual uint64_t CoreStep() = 0;
	virtual void ExecutePeripheral(uint64_t time_gap) = 0;
	virtual void SetMedium(Medium* medium) noexcept = 0;
};

class Medium : public boost::serialization::singleton<Medium> {
public:
	SIM_IMPORT_EXPORT Medium();
	SIM_IMPORT_EXPORT void AddController(IController* peripheral);
	SIM_IMPORT_EXPORT bool RemoveController(IController* controller);
	MediumState* GetState() noexcept { return &mstate_; }
private:
	template<typename Func>
	void pause_execution(Func&& fn) {
		if (running_.load(std::memory_order_relaxed) == false) { fn(); return; }
		
		cnt_paused_ = 0;
		paused_.store(true, std::memory_order_release);
		
		{
			std::unique_lock lk(mut_);
			cv_.wait(lk, [this]() { return cnt_paused_ == 2; }); // both threads stopped
		}
		
		fn();

		paused_.store(false, std::memory_order_release);
	}

	void wait_if_paused() {
		if (paused_.load(std::memory_order_acquire)) {
			{
				std::lock_guard lk(mut_);
				cnt_paused_++;
			}
			cv_.notify_one();
			while (paused_.load(std::memory_order_relaxed));
		}
	}

	void start() noexcept;
	void stop() noexcept;

	std::atomic_bool running_;
	std::atomic_bool paused_;

	int cnt_paused_;
	std::mutex mut_;
	std::condition_variable cv_;
	size_t controllers_count_;
	std::array<IController*, 10> controllers_;
	
	std::thread medium_thread_;
	std::thread core_thread_;
	int MediumProc() noexcept;
	int CoreProc() noexcept;
	double cpu_frequency_;
	MediumState mstate_;
};
