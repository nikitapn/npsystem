// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <queue>

#include "mapped.hpp"

namespace npdb {

template<typename T>
class _bfs_iterator {
	mapped_ptr<T> root_;
	mapped_ptr<T> current_;
	size_t current_level_;
	std::queue<std::pair<mapped_ptr<T>, size_t>> queue_;

	void iterate(mapped_ptr<T> node) {
		if (node->is_leaf() == true) return;

		auto next_level = current_level_ + 1;
		for (size_t i = 0; i <= node->n_keys_; ++i) {
			queue_.push({ mapped_ptr<T>(node->nodes_[i], root_.db()), next_level });
		}
	}
public:
	bool next() {
		if (queue_.empty()) return false;
		{
			auto x = queue_.front();
			current_ = std::move(x.first);
			current_level_ = x.second;
		}
		queue_.pop();
		iterate(current_);
		return true;
	}

	auto value() noexcept {
		return std::make_pair(current_.get(), current_level_);
	}

	_bfs_iterator(mapped_ptr<T> root)
		: root_(root)
		, current_{root}
		, current_level_(0)
	{
		iterate(root_);
	}
};

}