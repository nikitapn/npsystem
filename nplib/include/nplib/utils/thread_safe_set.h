#pragma once


#include <set>
#include <mutex>

template<typename Container, typename Pred>
void thread_safe_for_each(Container& c, Pred&& pred);

template<typename Key>
class thread_safe_set
{
	template<typename Container, typename Pred>
	friend void thread_safe_for_each(Container& c, Pred&& pred);
public:
	template<typename T>
	void insert(T&& key) {
		std::lock_guard<std::mutex> lk(mut_);
		data_.insert(std::forward<T>(key));
	}
	void erase(const Key& key) {
		std::lock_guard<std::mutex> lk(mut_);
		auto it = data_.find(key);
		if (it == data_.end())
			return;
		data_.erase(it);
	}
	void clear() {
		std::lock_guard<std::mutex> lk(mut_);
		data_.clear();
	}
	auto size() const noexcept { return data_.size(); }
private:
	mutable std::mutex mut_;
	std::set<Key> data_;
};

template<typename Container, typename Pred>
void thread_safe_for_each(Container& c, Pred&& pred) {
	std::lock_guard<std::mutex> lk(c.mut_);
	for (auto& i : c.data_)
		pred(i);
}