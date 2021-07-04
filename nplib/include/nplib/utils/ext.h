// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <tuple>
#include <type_traits>

namespace ext {

namespace detail {
template <typename T, typename Func, size_t... I>
void tuple_for_each_impl(T&& t, Func&& fn, std::index_sequence<I...>) {
	[[maybe_unused]] auto unused = { (fn(std::get<I>(std::forward<T>(t))), void(), true)... };
}

template <typename T, typename Tuple>
struct tuple_index;

template <typename T, typename... Types>
struct tuple_index<T, std::tuple<T, Types...>> {
    static constexpr std::size_t value = 0;
};
template <typename T, typename U, typename... Types>
struct tuple_index<T, std::tuple<U, Types...>> {
    static constexpr std::size_t value = tuple_index<T, std::tuple<Types...>>::value + 1;
};

template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T>
struct tuple_has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Types>
struct tuple_has_type<T, std::tuple<U, Types...>> : tuple_has_type<T, std::tuple<Types...>> {};

template <typename T, typename... Types>
struct tuple_has_type<T, std::tuple<T, Types...>> : std::true_type {};

} // namespace detail

template <typename T, typename Tuple>
constexpr bool tuple_has_type_v = detail::tuple_has_type<T, Tuple>::value;

template <typename T, typename Func>
void tuple_for_each(T&& t, Func&& fn){
	detail::tuple_for_each_impl(std::forward<T>(t), std::forward<Func>(fn),
		std::make_index_sequence<std::tuple_size<std::remove_reference_t<T>>::value>());
}

template<typename T, typename Tuple>
constexpr size_t tuple_index_v = detail::tuple_index<T, Tuple>::value;

} // namespace ext