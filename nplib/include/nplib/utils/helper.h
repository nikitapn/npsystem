#pragma once

#include <iterator>
#include <algorithm>
#include <assert.h>

namespace nplib::hlp {

template<size_t MaxSize, typename FwdIter, typename T, typename Comp = std::less<T>>
size_t insert_sort_desc(FwdIter begin, FwdIter end, const T& val, Comp cmp = std::less<T>()) {
	assert(std::distance(begin, end) < MaxSize);

	if (begin != end) {
		for (auto it = begin; it != end; it++) {
			if (cmp(*it, val)) {
				std::move(it, end, std::next(it));
				*it = val;
				return std::distance(begin, it);
			}
		}
		*end = val;
		return std::distance(begin, end);
	} else {
		*begin = val;
		return 0;
	}
}

template<size_t MaxSize, typename FwdIter, typename T, typename Comp = std::greater<T>>
size_t insert_sort_asc(FwdIter begin, FwdIter end, const T& val, Comp cmp = std::greater<T>()) {
	assert(std::distance(begin, end) < MaxSize);

	if (begin != end) {
		for (auto it = begin; it != end; it++) {
			if (cmp(*it, val)) {
				std::move(it, end, std::next(it));
				*it = val;
				return std::distance(begin, it);
			}
		}
		*end = val;
		return std::distance(begin, end);
	} else {
		*begin = val;
		return 0;
	}
}

} // namespace nplib:hlp
