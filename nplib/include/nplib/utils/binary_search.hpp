// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <functional>

namespace nplib {

template<class ForwardIt, class T, class Compare = std::less<>, class Equal = std::equal_to<>>
inline bool binary_find(ForwardIt first, ForwardIt last, const T& value, ForwardIt& out, 
	Compare less = {}, Equal equal = {})
{
	// Note: BOTH type T and the type after ForwardIt is dereferenced 
	// must be implicitly convertible to BOTH Type1 and Type2, used in Compare. 
	// This is stricter than lower_bound requirement (see above)
	out = std::lower_bound(first, last, value, less);
	return out != last && equal(*out, value);
}

template<class ForwardIt, class T, class Compare = std::less<>, class Equal = std::equal_to<>>
inline ForwardIt binary_find_it(ForwardIt first, ForwardIt last, const T& value,
	Compare less = {}, Equal equal = {})
{
	ForwardIt result;
	return binary_find(first, last, value, result, less, equal) ? result : last;
}

} // namespace nplib