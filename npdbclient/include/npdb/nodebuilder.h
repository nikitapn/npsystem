// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "constants.h"
#include "traits.h"

namespace odb {
namespace detail {
class node_builder {
public:
	template<typename Node, typename T>
	static Node create_new(T* obj) noexcept {
		return Node(obj);
	}

	template<typename Node>
	static Node create_assuming_there_wasnt_any(oid_t id, typename Node::type_t* obj) noexcept {
		assert(id != INVALID_ID);
		assert(obj);

		using policy = typename Node::policy;
		using type_t = typename Node::type_t;

		static_assert(!Node::is_static_id(), "This constructor is not allowed for static ID");

		if constexpr(has_modified_flag<type_t>::value) {
			obj->set_modified(false);
		}

		Node n;

		n.data_ = policy::make_pointer(id, F_REPLICATED, obj);
		policy::put(id, n.data_);

		if constexpr(has_self_node_member<type_t>::value) {
			obj->self_node = n;
		}

		return n;
	}

	template<typename Node, typename Ptr>
	static Node create_after_cast(oid_t tmp_id, uint32_t tmp_flags, Ptr&& ptr) {
		return Node(tmp_id, tmp_flags, std::move(ptr));
	}
};

}} //namespace odb::detail;