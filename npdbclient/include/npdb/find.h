// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once


#ifndef FIND_H_
#define FIND_H_

#include "db.h"

namespace odb::utils {

template<typename T>
auto find_by_name(T& list, const std::string& name) {
	static_assert(is_node_list<T>::value, "T is not a node list");
	static_assert(is_sys_tree_item<typename T::type_t>::value, "Node is not a system tree item");

	using node_t = typename T::node_t;

	list.fetch_all_nodes();
	for (auto& item : list) {
		if (item->get_name() == name) return std::optional<node_t>(item);
	}
	return std::optional<node_t>();
}

} // namespace odb:utils

#endif // FIND_H_