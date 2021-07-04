// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once


#include <type_traits>

namespace npsys {

class access {
public:
	template<class T>
	static auto& rw(const T& t) noexcept {
		return const_cast<std::remove_const_t<T>&>(t);
	}
};

}; // namespace npsys