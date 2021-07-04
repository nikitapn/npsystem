// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once


template <typename T, template <typename, bool> typename share_policy, oid_t Default_Id, typename S>
inline std::ostream& operator << (std::ostream& os,
	const odb::detail::shared_node_t<T, share_policy, Default_Id, S>& node) {
	os << "node: " << typeid(node).name() << " - id: " << node.id();
	if (node.loaded()) {
		os << " loaded at: " << std::hex << node.get() << std::endl << std::dec;
	}
	os << std::endl;
	return os;
}

template<typename NodeType, oid_t default_id, typename S>
inline std::ostream& operator << (std::ostream& os,
	const odb::detail::node_list_base<NodeType, default_id, S>& list) {
	os << "list: " << typeid(list).name() << " - id: " << list.id() << " :" << std::endl;
	for (auto& n : list) {
		os << "\t" << n;
	}
	return os;
}
