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