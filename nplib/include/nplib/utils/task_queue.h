// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <queue>
#include <array>
#include <vector>
#include <tuple>
#include <mutex>
#include <future>
#include <condition_variable>

namespace nplib {

class task_queue {
	using task_t = std::packaged_task<int()>;

	std::mutex mut_;
	std::condition_variable cv_;
	std::queue<task_t> tasks_;
	bool quit_ = false;
public:
	template<typename Func>
	std::future<int> add_task(Func&& fn) {
		task_t t(std::forward<Func>(fn));
		auto f = t.get_future();
		{
			std::lock_guard<std::mutex> lk(mut_);
			tasks_.push(std::move(t));
		}
		cv_.notify_one();
		return f;
	}

	enum class Result {
		Processed, Exit, Timeout
	};

	Result process_task_until(const std::chrono::steady_clock::time_point& deadline) noexcept {
		if (std::chrono::steady_clock::now() > deadline) return Result::Timeout;

		std::unique_lock<std::mutex> lk(mut_);
		
		if (!tasks_.empty()) {
			auto task = std::move(tasks_.front());
			tasks_.pop();
		
			lk.unlock();
			task();

			return Result::Processed;
		}
	
		if (cv_.wait_until(lk, deadline, [&]() { return !tasks_.empty() || quit_; })) {
			if (quit_) return Result::Exit;

			auto task = std::move(tasks_.front());
			tasks_.pop();
		
			lk.unlock();
			task();

			return Result::Processed;
		} else {
			return Result::Timeout;
		}
	}

	Result process_task() noexcept {
		std::unique_lock<std::mutex> lk(mut_);
		cv_.wait(lk, [this] { return quit_ || !tasks_.empty(); });

		if (quit_) return Result::Exit;

		auto task = std::move(tasks_.front());
		tasks_.pop();
		
		lk.unlock();
		task();

		return Result::Processed;
	}

	void exit() noexcept {
		{
			std::lock_guard<std::mutex> lk(mut_);
			quit_ = true;
		}
		cv_.notify_one();
	}
};

enum class PopResult {
	Exit, Value, Timeout
};

template<typename Ty>
class thread_safe_queue {
	mutable std::mutex mut_;
	std::condition_variable cv_;
	bool quit_ = false;
	std::queue<Ty> data_;
public:
	using type_t = Ty;

	template<typename T>
	void push(T&& value) noexcept {
		std::lock_guard<std::mutex> lk(mut_);
		data_.push(std::forward<T>(value));
		cv_.notify_one();
	}

	PopResult pop(std::chrono::steady_clock::time_point deadline, Ty& val) noexcept {
		if (std::chrono::steady_clock::now() > deadline) return PopResult::Timeout;

		PopResult result;
		std::unique_lock<std::mutex> lk(mut_);

		while (true) {
			auto status = cv_.wait_until(lk, deadline);
			if (status == std::cv_status::timeout) {
				result = (quit_ ? PopResult::Exit : PopResult::Timeout);
				break;
			} else { // (status == std::cv_status::no_timeout)
				if (quit_ == true) {
					result = PopResult::Exit;
					break;
				} else if (!data_.empty()) {
					val = move(data_.back());
					data_.pop();
					result = PopResult::Value;
					break;
				} else {
					// false awaking	
				}
			}
		}

		return result;
	}

	PopResult pop(Ty& val) noexcept {
		std::unique_lock<std::mutex> lk(mut_);
		cv_.wait(lk, [this] { return !data_.empty() || quit_; });

		if (quit_ == true) return PopResult::Exit;
		
		val = std::move(data_.back());
		data_.pop();
		return PopResult::Value;
	}

	void exit() noexcept {
		std::lock_guard<std::mutex> lk(mut_);
		quit_ = true;
		cv_.notify_one();
	}
};

template<typename T>
class thread_safe_vector {
	std::vector<T> data_;
	mutable std::mutex mut_;
public:
	void push_back(T& val) {
		std::lock_guard<std::mutex> lk(mut_);
		data_.push_back(val);
	}

	void push_back(T&& val) {
		std::lock_guard<std::mutex> lk(mut_);
		data_.push_back(std::move(val));
	}

	void erase(const T& val) {
		std::lock_guard<std::mutex> lk(mut_);
		auto it = std::find(data_.begin(), data_.end(), val);
		assert(it != data_.end());
		data_.erase(it);
	}

	void lock() const { mut_.lock(); }
	void unlock() const { mut_.unlock(); }
	auto begin() const { return data_.begin(); }
	auto end() const { return data_.end(); }
	auto cbegin() const { return data_.cbegin(); }
	auto cend() const { return data_.cend(); }
	/*
	void clear() noexcept {
		std::lock_guard<std::mutex> lk(mut_);
		data_.clear();
	}
	*/
};

} // namespace nplib


#endif // TASK_QUEUE_HPP