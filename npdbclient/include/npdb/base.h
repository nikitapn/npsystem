// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <type_traits>

namespace odb {
namespace detail {
class basic_node_abstract {
#ifdef DEBUG
public:
	virtual ~basic_node_abstract() = default; // for typid RTTI
#endif
};
class basic_list_abstract {};
}
struct no_interface {};
struct basic_serialization {};
struct special_serialization {};

template<typename T>
struct common_interface {
	using serialize_as = T;
	static constexpr auto has_common_interface() noexcept { return true; }
	virtual ~common_interface() = default;
};

template<>
struct common_interface<no_interface> {
	static constexpr auto has_common_interface() noexcept { return false; }
};

}// namespace odb