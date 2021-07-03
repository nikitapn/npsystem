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
	virtual double ExecuteCore() noexcept = 0;
	virtual void ExecutePeripheral(uint64_t time_gap) noexcept = 0;
	virtual void SetMedium(Medium* medium) noexcept = 0;
};

class Medium : public boost::serialization::singleton<Medium> {
public:
	SIM_IMPORT_EXPORT Medium();
	SIM_IMPORT_EXPORT void AddController(IController* peripheral);
	SIM_IMPORT_EXPORT bool RemoveController(IController* controller);
	MediumState* GetState() noexcept { return &mstate_; }
private:
	uint64_t GetCpuFrequency(uint64_t seconds = 1);
	template<typename Func>
	void Pause(Func fn) {
		if (running_ == false) {
			fn();
			return;
		}
		paused_ = 0;
		pause_ = true;
		{
			std::unique_lock lk(mut_);
			cv_.wait(lk, [this]() { return paused_ == 2; });
			fn();
		}
		{
			std::lock_guard lk(mut2_);
			pause_ = false;
		}
		cv2_.notify_all();
	}
	void start() noexcept;
	void stop() noexcept;

	int paused_;
	std::atomic_bool pause_;
	std::mutex mut_;
	std::condition_variable cv_;
	std::mutex mut2_;
	std::condition_variable cv2_;
	size_t controllers_count_;
	std::array<IController*, 10> controllers_;
	std::atomic_bool running_;
	std::thread medium_thread_;
	std::thread core_thread_;
	int MediumProc() noexcept;
	int CoreProc() noexcept;
	double cpu_frequency_;
	MediumState mstate_;
};