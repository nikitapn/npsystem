// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"

namespace npsys {

class Strings
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<Strings>
	, public odb::modified_flag
	, public odb::self_node_member<npsys::strings_n> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & name_;
		ar & items_;
	}
	std::vector<std::pair<size_t, std::string>> items_;
public:
	Strings() = default;
	
	template<typename T>
	Strings(T&& name) 
		: systree_item(std::forward<T>(name)) 
	{
	}
	auto begin() noexcept { return items_.begin(); }
	auto end() noexcept { return items_.end(); }

	const std::string* get(size_t number) const noexcept {
		for (auto& item : items_) {
			if (item.first == number) return &item.second;
		}
		return nullptr;
	}

	void set(size_t number, const std::string& value) noexcept {
		set_modified();
		for (auto& item : items_) {
			if (item.first == number) {
				item.second = value;
				return;
			}
		}
		items_.emplace_back(number, value);
	}

	void set(size_t old_number, size_t number, const std::string& value) noexcept {
		for (auto& item : items_) {
			if (item.first == old_number) {
				item.first = number;
				item.second = value;
				set_modified();
				return;
			}
		}
	}

	void remove(size_t number) {
		if (auto founded = std::find_if(items_.begin(), items_.end(), [number](auto& pair) {return pair.first == number; });
			founded != items_.end()) {
			items_.erase(founded);
			set_modified();
		}
	}
};

} // namespace npsys
