// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <variant>
#include <boost/serialization/access.hpp>
#include <boost/archive/archive_exception.hpp>
#include <nplib/utils/value_parser.h>

namespace
{
using variant_base = std::variant<bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, double, std::string, uint64_t, int64_t>;

class to_string_visitor {
public:
	std::string operator()(bool value) const {
		return value == true ? "1" : "0";
	}
	std::string operator()(uint8_t value) const {
		return std::to_string(value);
	}
	std::string operator()(int8_t value) const {
return std::to_string(value);
	}
	std::string operator()(uint16_t value) const {
return std::to_string(value);
	}
	std::string operator()(int16_t value) const {
return std::to_string(value);
	}
	std::string operator()(uint32_t value) const {
return std::to_string(value);
	}
	std::string operator()(int32_t value) const {
return std::to_string(value);
	}
	std::string operator()(uint64_t value) const {
return std::to_string(value);
	}
	std::string operator()(int64_t value) const {
return std::to_string(value);
	}
	std::string operator()(float value) const {
return std::to_string(value);
	}
	std::string operator()(double value) const {
return std::to_string(value);
	}
	std::string operator()(const std::string& value) const {
		return value;
	}
};

class from_string_visitor {
public:
	from_string_visitor(const std::string& str) : str_(str) {}

	void operator()(bool& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(uint8_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(int8_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(uint16_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(int16_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(uint32_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(int32_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(uint64_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(int64_t& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(float& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(double& value) const {
		ParseTextValue(str_.c_str(), str_.length(), value);
	}
	void operator()(std::string& value) const {
		value = str_;
	}
private:
	const std::string& str_;
};
}

namespace boost {
namespace serialization {
namespace detail {

template<typename T>
struct variant_type_to_index;

template<>
struct variant_type_to_index<bool> {
	static constexpr uint8_t value = 0;
};
template<>
struct variant_type_to_index<uint8_t> {
	static constexpr uint8_t value = 1;
};
template<>
struct variant_type_to_index<int8_t> {
	static constexpr uint8_t value = 2;
};
template<>
struct variant_type_to_index<uint16_t> {
	static constexpr uint8_t value = 3;
};
template<>
struct variant_type_to_index<int16_t> {
	static constexpr uint8_t value = 4;
};
template<>
struct variant_type_to_index<uint32_t> {
	static constexpr uint8_t value = 5;
};
template<>
struct variant_type_to_index<int32_t> {
	static constexpr uint8_t value = 6;
};
template<>
struct variant_type_to_index<float> {
	static constexpr uint8_t value = 7;
};
template<>
struct variant_type_to_index<double> {
	static constexpr uint8_t value = 8;
};
template<>
struct variant_type_to_index<std::string> {
	static constexpr uint8_t value = 9;
};
template<>
struct variant_type_to_index<uint64_t> {
	static constexpr uint8_t value = 10;
};
template<>
struct variant_type_to_index<int64_t> {
	static constexpr uint8_t value = 11;
};

template<class Archive>
class serialize_visitor
{
public:
	serialize_visitor(Archive& _ar) : ar(_ar) { }
	template<typename T>
	void operator()(const T& value) const
	{
		ar << variant_type_to_index<T>::value;
		ar << value;
	}
	Archive& ar;
};
} // namespace detail

template<class Archive>
inline void save(Archive & ar, const variant_base& v, const unsigned int /*file_version*/) {
	std::visit(detail::serialize_visitor<Archive>(ar), v);
}

template<class Archive>
inline void load(Archive & ar, variant_base& v, const unsigned int /*file_version*/) {
	uint8_t index;
	ar >> index;
	switch (index) {
	case 0: {
		bool val;
		ar >> val;
		v = val;
		break;
	}
	case 1: {
		uint8_t val;
		ar >> val;
		v = val;
		break;
	}
	case 2: {
		int8_t val;
		ar >> val;
		v = val;
		break;
	}
	case 3: {
		uint16_t val;
		ar >> val;
		v = val;
		break;
	}
	case 4: {
		int16_t val;
		ar >> val;
		v = val;
		break;
	}
	case 5: {
		uint32_t val;
		ar >> val;
		v = val;
		break;
	}
	case 6: {
		int32_t val;
		ar >> val;
		v = val;
		break;
	}
	case 7: {
		float val;
		ar >> val;
		v = val;
		break;
	}
	case 8: {
		double val;
		ar >> val;
		v = val;
		break;
	}
	case 9: {
		std::string val;
		ar >> val;
		v = val;
		break;
	}
	case 10: {
		uint64_t val;
		ar >> val;
		v = val;
		break;
	}
	case 11: {
		int64_t val;
		ar >> val;
		v = val;
		break;
	}
	default:
		throw boost::archive::archive_exception(boost::archive::archive_exception::input_stream_error);
	}
}
template<class Archive>
inline void serialize(Archive & ar, variant_base& v, const unsigned int file_version) {
	boost::serialization::split_free(ar, v, file_version);
}
} // namespace serialization
} // namespace boost

namespace npsys {
class CMyVariant {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & value_;
	}
public:
	CMyVariant() = default;

	template<typename T>
	CMyVariant(const T& value) {
		_copy(value);
	}

	template<typename T>
	CMyVariant& operator = (const T& value) {
		_copy(value);
		return *this;
	}

	template<typename T>
	const T& as() const {
		return std::get<T>(value_);
	}

	template<typename T>
	CMyVariant(CMyVariant&& value) {
		_move(value);
	}

	template<typename T>
	CMyVariant& operator = (T&& value) {
		_move(value);
		return (*this);
	};

	std::string ToString() const noexcept {
		return std::visit(to_string_visitor(), value_);
	}
	void Parse(const std::string& str) {
		std::visit(from_string_visitor(str), value_);
	}
	auto which() const {
		return value_.index();
	}
private:
	template<typename T>
	std::enable_if_t<std::is_same_v<T, CMyVariant>, void>
		_copy(const T& other) {
		value_ = other.value_;
	}
	template<typename T>
	std::enable_if_t<!std::is_same_v<T, CMyVariant>, void>
		_copy(const T& value) {
		value_ = value;
	}

	template<typename T>
	std::enable_if_t<std::is_same_v<T, CMyVariant>, void>
		_move(T&& other) {
		value_ = std::move(other.value_);
	}
	template<typename T>
	std::enable_if_t<!std::is_same_v<T, CMyVariant>, void>
		_move(T&& value) {
		value_ = std::move(value);
	}

	variant_base value_;
};
} // namespace npsys
