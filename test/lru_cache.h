// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once


#include <type_traits>
#include <utility>
//#include <concepts>
#include <assert.h>

/*
template<class T>
concept LRUCacheKey = requires(
	const std::remove_reference_t<T> & t,
	const std::remove_reference_t<T> & u) {
		{ t == u };
};
*/

template<typename TKey, typename TData, size_t MAX_SIZE>
class LRUCache {
	static_assert(MAX_SIZE >= 2);

	struct Entry {
		Entry* next;
		Entry* prev;
		std::pair<TKey, TData> kv;
	};

	std::aligned_storage_t<sizeof(Entry), alignof(Entry)> storage_[MAX_SIZE];
	Entry* last_used_ = nullptr;
	Entry* least_used_ = nullptr;
	size_t last_index_ = 0;

	Entry* operator[](size_t ix) noexcept {
		return std::launder(reinterpret_cast<Entry*>(&storage_[ix]));
	}

	Entry* find(const TKey& key) noexcept {
		auto entry = last_used_;
		while (entry) {
			if (entry->kv.first == key) return entry;
			entry = entry->prev;
		}
		return nullptr;
	}
public:
	TData* get(const TKey& key) noexcept {
		auto entry = find(key);

		if (entry == nullptr) return nullptr;
		if (last_used_ == entry) return &entry->kv.second;

		if (least_used_ == entry) {
			if (least_used_->next) [[likely]] least_used_->next->prev = nullptr;
			least_used_ = least_used_->next;
		}

		assert(last_used_->next == nullptr);

		if (entry->prev) [[likely]] entry->prev->next = entry->next;
		entry->next->prev = entry->prev;

		entry->prev = last_used_;
		entry->next = nullptr;
		
		last_used_->next = entry;
		last_used_ = entry;

		return &entry->kv.second;
	}

	template<typename... Args>
	TData& emplace(const TKey& key, Args&&... args) noexcept {
		assert(find(key) == nullptr);
		
		Entry* entry;
		
		if (last_index_ < MAX_SIZE) {
			entry = this->operator[](last_index_++);
			if (least_used_ == nullptr) {
				least_used_ = last_used_ = entry;
				entry->kv.first = key;
				new (&entry->kv.second) TData(std::forward<Args>(args)...);
				entry->prev = nullptr;
				entry->next = nullptr;
				return entry->kv.second;
			} 
		} else { // evict least_used_
			least_used_->kv.second.~TData();
			entry = least_used_;
			least_used_->next->prev = nullptr;
			least_used_ = least_used_->next;
		}
		
		entry->kv.first = key;
		new (&entry->kv.second) TData(std::forward<Args>(args)...);

		last_used_->next = entry;

		entry->next = nullptr;
		entry->prev = last_used_;

		last_used_ = entry;

		return entry->kv.second;
	}

	struct iterator {
		Entry* ptr_;
		
		iterator& operator++() {
			ptr_ = ptr_->next;
			return (*this);
		}

		iterator& operator++(int) {
			iterator tmp{this};
			ptr_ = ptr_->next;
			return tmp;
		}

		std::pair<TKey, TData>* operator->() { 
			assert(ptr_);
			return &ptr_->kv;
		}

		std::pair<TKey, TData>& operator*() { 
			assert(ptr_);
			return ptr_->kv;
		}

		bool operator!=(const iterator& other) { return ptr_ != other.ptr_; }

		iterator(Entry& ptr) : ptr_{ptr} {}
	};

	iterator begin() { return iterator{last_used_}; }
	iterator end() { return iterator{least_used_}; }

	LRUCache() = default;
	
	LRUCache(const LRUCache&) = delete;
	LRUCache& operator == (const LRUCache&) = delete;
	
	LRUCache(LRUCache&&) = delete;
	LRUCache& operator == (LRUCache&&) = delete;
	
	~LRUCache() {
		for (size_t i = 0; i < last_index_; ++i) {
			this->operator[](i)->kv.second.~TData();
		}
	}
};