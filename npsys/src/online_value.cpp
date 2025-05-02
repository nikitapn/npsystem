// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <assert.h>
#include <nplib/utils/format.h>
#include <npsys/other/online_value.h>
#include <variant>
#include <sstream>

bool online_value::is_quality_not_good() const noexcept {
	if (val_.s == nps::var_status::VST_DEVICE_NOT_RESPONDED
		|| val_.s == nps::var_status::VST_UNKNOWN) {
		return true;
	}

	if (is_quality() == false) return false;
	
	switch (variable::GetClearType(type_)) {
	case type_t::NPT_BOOL:
		return to_Qbit().quality != VQ_GOOD;
	case type_t::NPT_U8:
		return to_Qu8().quality != VQ_GOOD;
	case type_t::NPT_I8:
		return to_Qi8().quality != VQ_GOOD;
	case type_t::NPT_U16:
		return to_Qu16().quality != VQ_GOOD;
	case type_t::NPT_I16:
		return to_Qi16().quality != VQ_GOOD;
	case type_t::NPT_U32:
		return to_Qu32().quality != VQ_GOOD;
	case type_t::NPT_I32:
		return to_Qi32().quality != VQ_GOOD;
	case type_t::NPT_F32:
		return to_Qflt().quality != VQ_GOOD;
	default:
		return true;
	};
}

void trim(std::string& s) {
	auto pos = s.rfind('.');
	if ((pos != std::string::npos) && (s.length() - pos > 4)) {
		s.erase(pos + 4);
	}
}

std::string online_value::to_string_impl(int /* precision */, bool show_quality) const noexcept {
	std::string result;
	using namespace npsystem::types;
	try
	{
		switch (val_.s)
		{
		case nps::var_status::VST_UNKNOWN:
			result = "unk";
			break;
		case nps::var_status::VST_DEVICE_NOT_RESPONDED:
			result = "nc";
			break;
		default:
			switch (variable::GetClearType(type_))
			{
			case type_t::NPT_BOOL:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QBIT& bt = to_Qbit();
						result = (bt.quality != VQ_GOOD) ? 'x' : (bt.value == NPSYSTEM_TRUE ? '1' : '0');
					} else {
						const QBIT& bt = to_Qbit();
						result = (bt.value == NPSYSTEM_TRUE ? "0b1, " : "0b0, ");
						result += (bt.quality == VQ_GOOD ? "0b1" : "0b0");
					}
				} else {
					result = to_bit().value == NPSYSTEM_TRUE ? '1' : '0';
				}
				break;
			case type_t::NPT_U8:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU8& val = to_Qu8();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU8& val = to_Qu8();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u8().value);
				}
				break;
			case type_t::NPT_I8:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI8& val = to_Qi8();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI8& val = to_Qi8();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i8().value);
				}
				break;
			case type_t::NPT_U16:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU16& val = to_Qu16();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU16& val = to_Qu16();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u16().value);
				}
				break;
			case type_t::NPT_I16:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI16& val = to_Qi16();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI16& val = to_Qi16();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i16().value);
				}
				break;
			case type_t::NPT_U32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU32& val = to_Qu32();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU32& val = to_Qu32();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u32().value);
				}
				break;
			case type_t::NPT_I32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI32& val = to_Qi32();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI32& val = to_Qi32();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i32().value);
				}
				break;
			case type_t::NPT_F32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const auto& val = to_Qflt();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
							trim(result);
						}
					} else {
						const auto& val = to_Qflt();
						result = std::to_string(val.value);
						trim(result);
						result += ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					const auto& val = to_flt();
					result = std::to_string(val.value);
					trim(result);
				}
				break;
			default:
				result = '?';
				break;
			}
			break;
		};
	}
	catch (std::bad_cast&)
	{
	}
	catch (...)
	{
	}
	return result;
}



#include <boost/tti/has_member_data.hpp>

BOOST_TTI_HAS_MEMBER_DATA(quality);

template<typename T>
inline void from_boolean_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += (val.value == NPSYSTEM_TRUE ? "true" : "false");
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';
	} else {
		out = "value=";
		out += (val.value == NPSYSTEM_TRUE ? "true" : "false");
	}
}

template<typename T>
inline void from_int_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += std::to_string(val.value) + 'i';
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';
	} else {
		out = "value=";
		out += std::to_string(val.value) + 'i';
	}
}

template<typename T>
inline void from_float_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += std::to_string(val.value);
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';;
	} else {
		out = "value=";
		out += std::to_string(val.value);
	}
}

std::string online_value::to_influx_db() const noexcept {
	std::string result;
	try {
		switch (val_.s) {
		case nps::var_status::VST_UNKNOWN:
		case nps::var_status::VST_DEVICE_NOT_RESPONDED:
			break;
		default:
			switch (variable::GetClearType(type_)) {
			case type_t::NPT_BOOL:
				if (variable::IsQuality(type_)) {
					const auto& val = to_Qbit();
					from_boolean_to_influx(val, result);
				} else {
					const auto& val = to_bit();
					from_boolean_to_influx(val, result);
				}
				break;
			case type_t::NPT_U8:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qu8();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u8();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I8:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qi8();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i8();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_U16:
			if (variable::IsQuality(type_)) {
						const auto& val = to_Qu16();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u16();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I16:
			if (variable::IsQuality(type_)) {
						const auto& val = to_Qi16();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i16();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_U32:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qu32();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u32();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I32:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qi32();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i32();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_F32:
				if (variable::IsQuality(type_)) {
					const auto& val = to_Qflt();
					from_float_to_influx(val, result);
				} else {
					const auto& val = to_flt();
					from_float_to_influx(val, result);
				}
				break;
			default:
				break;
			}
			break;
		};
	}
	catch (std::bad_cast&)
	{
	}
	catch (...)
	{
	}
	return result;
}
