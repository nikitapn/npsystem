// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <npdb/db.h>

extern int test_smart_pointers();
extern int test_sim();
extern int test_memento();
extern int test_db();
extern int test_compiler();
extern int test_tsdb();
extern int test_stuff();
extern int test_storage();

int main() {
	SetConsoleOutputCP(CP_UTF8);
	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
	setvbuf(stderr, nullptr, _IOFBF, 1000);

	return test_compiler();
}


struct A { void fooA() {}}; 
struct B { void fooB() {}};


struct t1 {};
struct t2 {};
struct t3 {};

template<class T, typename = void, typename...>
struct trait_to_type;

template<class T, typename... TT>
struct trait_to_type<T, t1, TT...> {
	using type = A;
};

template<class T, typename... TT>
struct trait_to_type<T, t2, TT...> {
	using type = B;
};

template<class T, typename... Traits>
struct traits : public trait_to_type<T, Traits>::type... {
	using traits_ = std::tuple<Traits...>;
};

template <typename T, typename = void>
struct has_traits : std::false_type {};
template <typename T>
struct has_traits<T, std::void_t<decltype(T::traits_)>> : std::true_type {};

class C : public traits<C, t1, t2> {};

void ttt() {
	constexpr auto xxx = std::is_base_of_v<A, C>;
	using t = C::traits_;
	
	constexpr auto x = odb::stdext::tuple_contains_type_v<t2, t>;
}