#include "stdafx.h"
#include <vector>
#include <array>
#include <tuple>
#include <memory>
#include <stdlib.h>

/*
template<typename T = void>
class raw_ptr;

template<template <typename> typename PointerType, typename I, typename... T>
class PolyVector {
	template<typename T, typename First, typename... Types>
	static constexpr size_t type_to_index_t(size_t index = 0) {
		if constexpr (std::is_same_v<T, First>) return index;
		else return type_to_index_t<T, Types...>(index + 1);
	}
	template<typename U>
	static constexpr size_t type_to_index() {
		return type_to_index_t<U, T...>();
	}

	template<typename T>
	using Ty = std::conditional_t<std::is_same_v<PointerType<void>, raw_ptr<void>>, T*, PointerType<T>>;

	static constexpr auto types_n = sizeof... (T);
	std::array<size_t, types_n> types_count_;
	std::vector<Ty<I>> items_;

	template<typename U>
	constexpr auto cast_it(typename std::vector<Ty<I>>::iterator& it) { 
		return reinterpret_cast<typename std::vector<Ty<U>>::iterator&>(it);
	}

	template<typename U>
	constexpr auto cast_it_back(typename U& it) { 
		return reinterpret_cast<typename std::vector<Ty<I>>::iterator&>(it);
	}

	template<typename U>
	size_t get_type_index() {
		size_t count = 0;
		std::for_each_n(types_count_.begin(), type_to_index<U>(), [&count](size_t n) { count += n; });
		return count;
	}
public:
	auto begin() {
		return items_.begin();
	}
	
	auto end() {
		return items_.end();
	}
	
	template<typename U>
	auto begin_t() {
		constexpr auto type_index = type_to_index<U>();
		
		if (types_count_[type_index] == 0) {
			return cast_it<U>(items_.end());
		}

		auto it = cast_it<U>(items_.begin());
		std::advance(it, get_type_index<U>());
		return it;
	}

	template<typename U>
	auto end_t() {
		constexpr auto type_index = type_to_index<U>();
		auto count = types_count_[type_index];

		auto it = cast_it<U>(items_.begin());
		std::advance(it, get_type_index<U>() + count);
		return it;
	}

	template<typename U, std::enable_if_t<std::is_pointer_v<U>, void*> = nullptr>
	void push_back(U obj) {
		items_.insert(cast_it_back(end_t<std::remove_pointer_t<U>>()), obj);
		types_count_[type_to_index<std::remove_pointer_t<U>>()]++;
	}

	template<typename U, std::enable_if_t<!std::is_pointer_v<U>, void*> = nullptr>
	void push_back(U&& obj) {
		items_.insert(cast_it_back(end_t<typename U::element_type>()), std::forward<U>(obj));
		types_count_[type_to_index<typename U::element_type>()]++;
	}
};

struct Interface {
	virtual void print() = 0;
	virtual ~Interface() { std::cout << "~Interface()\n"; }
};

struct Thing1 : Interface {
	virtual void print() { std::cout << "Thing1\n"; }
};

struct Thing2 : Interface {
	virtual void print() { std::cout << "Thing2\n"; }
};

struct Thing3 : Interface {
	virtual void print() { std::cout << "Thing3\n"; }
};


void unique_test() {
	PolyVector<std::unique_ptr, Interface, Thing1, Thing2, Thing3> v;

	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing2>());
	v.push_back(std::make_unique<Thing3>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing2>());
	v.push_back(std::make_unique<Thing3>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing2>());
	v.push_back(std::make_unique<Thing2>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing3>());
	v.push_back(std::make_unique<Thing1>());
	v.push_back(std::make_unique<Thing2>());
	
	for (auto& thing : v) {
		thing->print();
	}

	std::cout << '\n';

	for (auto it = v.begin_t<Thing3>(); it != v.end_t<Thing3>(); it++) {
		(*it)->print();
	}
}
*/

int test_stuff() {
	/*
	PolyVector<raw_ptr, Interface, Thing1, Thing2, Thing3> v;

	v.push_back(new Thing1);
	v.push_back(new Thing1);
	v.push_back(new Thing2);
	v.push_back(new Thing3);
	v.push_back(new Thing1);
	v.push_back(new Thing2);
	v.push_back(new Thing3);
	v.push_back(new Thing1);
	v.push_back(new Thing2);
	v.push_back(new Thing2);
	v.push_back(new Thing1);
	v.push_back(new Thing1);
	v.push_back(new Thing3);
	v.push_back(new Thing1);
	v.push_back(new Thing2);
	

	for (auto& thing : v) {
		thing->print();
	}

	std::cout << '\n';

	for (auto it = v.begin_t<Thing3>(); it != v.end_t<Thing3>(); it++) {
		(*it)->print();
	}

	std::cout << "\nsmart pointers:\n\n";

	unique_test();
	*/

	return 0;
}