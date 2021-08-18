// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifdef _WIN32 
# include <Windows.h>
#	include <Shlobj_core.h>
#endif

#include <tuple>
#include <string>
#include <string_view>

#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/include/define_assoc_struct.hpp>
#include <boost/fusion/include/adapt_assoc_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/integer_sequence.hpp>
#include <boost/mp11/list.hpp>
#include <filesystem>
#include "colored_cout.h"

namespace nplib::config {

#ifdef _WIN32
// returns ../x64
inline std::filesystem::path GetExecutableRootPath() noexcept {
	auto buffer = std::make_unique<char[]>(MAX_PATH);
	int len = ::GetModuleFileNameA(NULL, buffer.get(), MAX_PATH);

	auto c = buffer.get() + len - 1;
	while (*c != L'\\') c--; c--;
	while (*c != L'\\') c--;

	assert(c > buffer.get());

	return std::string(buffer.get(), c - buffer.get() + 1);
}
#else
inline std::filesystem::path GetExecutableRootPath() noexcept {
	assert(false);
	return {};
}
#endif

namespace detail {
template<typename T>
struct sec;
} // namespace detail

template<typename Section, typename T>
struct ValueTrait {
public:
	using section = Section;
	using type_t = T;

	constexpr ValueTrait(const T& t) noexcept
		: default_value(t) {}

	const T default_value;
};

template<typename Section>
struct ValueTrait<Section, std::string> {
public:
	using section = Section;
	using type_t = std::string;

	constexpr ValueTrait(const std::string_view& t) noexcept
		: default_value(t) {}

	const std::string_view default_value;
};

namespace fuz = boost::fusion;
namespace mp11 = boost::mp11;

template<typename T>
struct config_base {
	std::filesystem::path config;
	std::filesystem::path data_dir;

	T* This() noexcept { return static_cast<T*>(this); }

	void load(std::string_view module_name) noexcept {
#ifdef _WIN32
		PWSTR ppszPath;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &ppszPath);
		assert(hr == S_OK);
		auto len = static_cast<int>(wcslen(ppszPath));

		std::filesystem::path roaming_path(ppszPath);
		CoTaskMemFree(ppszPath);

		data_dir = roaming_path;
		data_dir += L"\\npsystem";

		config = data_dir;
		config /= module_name;
		config += L".cfg";
#elif __linux__
		data_dir = "/var/local/npsystem";
		config = "/etc/npsystem";
		config /= module_name;
		config += ".cfg";
#else
#	error "platform is not supported"
#endif

		std::ifstream stream(config);

		auto this_ = This();

		if (!stream) {
			mp11::mp_for_each<mp11::mp_iota_c<fuz::result_of::size<T>::value>>([&](auto I) {
				fuz::at_c<static_cast<int>(I)>(*this_) = std::get<I>(this_->traits).default_value;
				});
			std::cerr << clr::color::red << "config file is missing: " << clr::color::reset << config << '\n';
		} else {
			boost::property_tree::ptree pt;
			boost::property_tree::ini_parser::read_ini(stream, pt);
			mp11::mp_for_each<mp11::mp_iota_c<fuz::result_of::size<T>::value>>([&](auto I) {
				auto name = std::string(detail::sec<typename std::tuple_element_t<I, decltype(T::traits)>::section>::name) + '.' +
					std::string(fuz::extension::struct_member_name<T, static_cast<int>(I)>::call());
				auto& val = fuz::at_c<static_cast<int>(I)>(*this_);
				try {
					val = pt.get<std::remove_reference_t<decltype(val)>>(name);
				} catch (...) {
					std::cerr << "error while reading: " << name << " default value will be using.\n";
					val = std::get<I>(this_->traits).default_value;
				}
				});
		}
	}
};

} // namespace nplib::config

template<typename T>
std::ostream& operator<<(std::ostream& os, const nplib::config::config_base<T>& cfg) {
	namespace fuz = boost::fusion;
	namespace mp11 = boost::mp11;
	os << "Config:\n";
	os << "config" << " = " << cfg.config << '\n';
	os << "data_dir" << " = " << cfg.data_dir << '\n';
	mp11::mp_for_each<mp11::mp_iota_c<fuz::result_of::size<T>::value>>([&](auto I) {
		auto name = std::string(fuz::extension::struct_member_name<T, static_cast<int>(I)>::call());
		auto const& val = fuz::at_c<static_cast<int>(I)>(static_cast<const T&>(cfg));
		os << name << " = " << val << '\n';
	});
	return os;
}

#define NPLIB_MAKE_DEFAULT_VALUE_TYPE(R, ATTRIBUTE_TUPLE_SIZE, I, ATTRIBUTE) \
	BOOST_PP_COMMA_IF(I) nplib::config::ValueTrait<nplib::config::sections::BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_TUPLE_ELEM(3, 2, ATTRIBUTE)), BOOST_PP_TUPLE_ELEM(3, 0, ATTRIBUTE)>

#define NPLIB_MAKE_DEFAULT_VALUE(R, ATTRIBUTE_TUPLE_SIZE, I, ATTRIBUTE) \
	BOOST_PP_COMMA_IF(I) BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_TUPLE_ELEM(3, 2, ATTRIBUTE))

#define BOOST_FUSION_DEFINE_STRUCT_PLUS_ENUM_IMPL(																			\
    NAMESPACE_SEQ, NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPLE_SIZE)													\
																																												\
BOOST_FUSION_ADAPT_STRUCT_NAMESPACE_DEFINITION_BEGIN(NAMESPACE_SEQ)											\
																																												\
    struct NAME : nplib::config::config_base<NAME>																											\
    {																																										\
		typedef NAME self_type;																															\
		static constexpr std::tuple<BOOST_PP_SEQ_FOR_EACH_I_R(1,														\
				NPLIB_MAKE_DEFAULT_VALUE_TYPE,																									\
				ATTRIBUTE_TUPLE_SIZE,																														\
				BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ))> 																						\
				traits																																					\
				{BOOST_PP_SEQ_FOR_EACH_I_R(1,																										\
					NPLIB_MAKE_DEFAULT_VALUE,																											\
					ATTRIBUTE_TUPLE_SIZE,																													\
					BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ))};																					\
																																												\
        BOOST_PP_IF(BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ATTRIBUTES_SEQ)),										\
            BOOST_FUSION_DEFINE_NONEMPTY_STRUCT_IMPL,																		\
            BOOST_FUSION_DEFINE_EMPTY_STRUCT_IMPL)(																			\
                NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPLE_SIZE)															\
    };																																									\
																																												\
    BOOST_FUSION_ADAPT_STRUCT_NAMESPACE_DEFINITION_END(NAMESPACE_SEQ)



#define BOOST_FUSION_ADAPT_STRUCT_ATTRIBUTES_FILLER_OP_MY(r, unused, elem)							\
        BOOST_PP_IIF(BOOST_FUSION_PP_IS_SEQ(elem),																			\
            BOOST_PP_CAT(BOOST_FUSION_ADAPT_ASSOC_STRUCT_FILLER_0 elem ,_END),					\
            BOOST_PP_EXPR_IIF(BOOST_PP_COMPL(BOOST_PP_IS_EMPTY(elem)),									\
                BOOST_FUSION_ADAPT_STRUCT_WRAP_ATTR(auto, elem)))

#define BOOST_FUSION_ADAPT_STRUCT_ATTRIBUTES_FILLER_MY(VA_ARGS_SEQ)											\
        BOOST_PP_SEQ_PUSH_FRONT(																												\
            BOOST_PP_SEQ_FOR_EACH(																											\
                BOOST_FUSION_ADAPT_STRUCT_ATTRIBUTES_FILLER_OP_MY,											\
                unused, VA_ARGS_SEQ),																										\
            (0,0))

#define BOOST_FUSION_ADAPT_STRUCT_MY(...)																								\
        BOOST_FUSION_ADAPT_STRUCT_BASE(																									\
            (0),																																				\
            (0)(BOOST_PP_SEQ_HEAD(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))),							\
            struct_tag,																																	\
            0,																																					\
            BOOST_FUSION_ADAPT_STRUCT_ATTRIBUTES_FILLER_MY(															\
              BOOST_PP_SEQ_TAIL(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))),								\
            BOOST_FUSION_ADAPT_STRUCT_C)

#define NPLIB_DEFINE_CONFIG_STRUCT(NAMESPACE_SEQ, NAME, ATTRIBUTES)											\
    BOOST_FUSION_DEFINE_STRUCT_PLUS_ENUM_IMPL(																					\
        (0)NAMESPACE_SEQ,																																\
        NAME,																																						\
        BOOST_PP_CAT(BOOST_FUSION_DEFINE_ASSOC_STRUCT_FILLER_0(0,0,0)ATTRIBUTES,_END),	\
        3)																																							\
																																												\
    BOOST_FUSION_ADAPT_STRUCT_MY(																												\
        BOOST_FUSION_ADAPT_STRUCT_NAMESPACE_DECLARATION((0)NAMESPACE_SEQ) NAME,					\
        ATTRIBUTES)


#define NPLIB_REGISTER_SECTION(section) \
namespace nplib::config { \
namespace sections { \
struct section; \
} \
namespace detail { \
template<> \
struct sec<sections::section> { \
	static constexpr const char* name = #section; \
}; \
}}

NPLIB_REGISTER_SECTION(General);