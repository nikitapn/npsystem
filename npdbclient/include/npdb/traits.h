// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <type_traits>

namespace odb {

template <typename T, typename = void>
struct has_self_node_member : std::false_type {};

template <typename T>
struct has_self_node_member <T, decltype(std::declval<T>().self_node, void())> : std::true_type {};

template <typename T, typename = void>
struct has_modified_flag : std::false_type {};

template <typename T>
struct has_modified_flag <T, decltype(std::declval<T>().is_modified(), void())> : std::true_type {};

template <typename T, typename = void>
struct has_observable_remove : std::false_type {};

template <typename T>
struct has_observable_remove <T, decltype(std::declval<T>().sig_node_removed(), void())> : std::true_type {};


namespace stdext {
namespace detail {
template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};
};

template <typename T, typename Tuple>
using tuple_contains_type = detail::has_type<T, Tuple>;

template <typename T, typename Tuple>
using tuple_contains_type_t = typename tuple_contains_type<T, Tuple>::type;

template <typename T, typename Tuple>
constexpr auto tuple_contains_type_v = tuple_contains_type_t<T, Tuple>::value;

}
} // namespace odb