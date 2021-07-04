// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <memory>
#include <tuple>
#include <nplib/utils/ext.h>

namespace odb {
class IMemento {
public:
	virtual ~IMemento() = default;
	virtual void Restore() const noexcept = 0;
};

class MementoCommonSupport {
//	friend class MementoManager;
//	template<class>
//	friend class NodeMementoManager;
public:
	std::unique_ptr<odb::IMemento> CreateMemento() noexcept { return CreateMemento_Common(); }
	virtual std::unique_ptr<IMemento> CreateMemento_Common() noexcept = 0;
};

template<typename T, typename... Fields>
class Memento : public IMemento {
	template<typename U>
	class Value {
		U& orig_;
		const U value_;
	public:
		Value(U& value) : orig_(value), value_(value) {}
		void Restore() const noexcept { orig_ = value_; }
	};
public:
	using tuple_t = std::tuple<Value<Fields>...>;
	Memento(T& obj, Fields&... fields)
		: obj_(obj)
		, fields_(
			std::make_tuple(Value<Fields>(fields)...)
		)
	{
	}
	virtual void Restore() const noexcept final {
		ext::tuple_for_each(fields_, [](auto& value) { value.Restore(); });
	}
private:
	T & obj_;
	tuple_t fields_;
};

template<typename T, typename... Fields>
decltype(auto) MakeMemento(T& obj, Fields&... fields) {
	return std::make_unique<Memento<T, Fields...>>(obj, fields...);
}
} // namespace odb