#pragma once

#include "base.h"
#include <istream>
#include <ostream>
#include <boost/archive/binary_iarchive_impl.hpp>
#include <boost/archive/binary_oarchive_impl.hpp>
#include <boost/archive/detail/register_archive.hpp>
#include <boost/config.hpp>
#include "constants.h"
#include "nodebuilder.h"
#include <unordered_set>
#include <unordered_map>

namespace boost {
namespace archive {
class binary_special {};
class binary_iarchive_special 
	: public binary_iarchive_impl<binary_iarchive_special, std::istream::char_type, std::istream::traits_type>
	, public binary_special {
public:
	using inherited = binary_iarchive_impl<
		binary_iarchive_special, std::istream::char_type, std::istream::traits_type>;

	binary_iarchive_special(std::istream & is, unsigned int flags = 0) :
		inherited(is, flags) {}
	binary_iarchive_special(std::streambuf & bsb, unsigned int flags = 0) :
		inherited(bsb, flags) {}

	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_node_abstract, T>,
		binary_iarchive_special>::type&
		operator&(T& t) {
		return serialize_node(t);
	}
	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_list_abstract, T>,
		binary_iarchive_special>::type&
		operator&(T & t) {
		return serialize_list(t);
	}
	template<class T>
	typename std::enable_if<
		!std::disjunction_v<
			std::is_base_of<odb::detail::basic_node_abstract, T>,
			std::is_base_of<odb::detail::basic_list_abstract, T>
		>, binary_iarchive_special>::type&
		operator&(T& t) {
		return inherited::operator>>(t);
	}

	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_node_abstract, T>,
		binary_iarchive_special>::type&
		operator>>(T& t) {
		return serialize_node(t);
	}
	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_list_abstract, T>,
		binary_iarchive_special>::type&
		operator>>(T & t) {
		return serialize_list(t);
	}
	template<class T>
	typename std::enable_if<
		!std::disjunction_v<
		std::is_base_of<odb::detail::basic_node_abstract, T>,
		std::is_base_of<odb::detail::basic_list_abstract, T>
		>, binary_iarchive_special>::type&
		operator>>(T& t) {
		return inherited::operator>>(t);
	}
	template<class T>
	void serialize_default(T& t) {
		inherited::operator>>(t);
	}
protected:
	std::unordered_map<uint64_t, void*> nodes_;
	// special node serialization 
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::special_serialization, T>,
		binary_iarchive_special>::type&
	serialize_node(T& t) {
		using type_t = typename T::type_t;
		uint64_t key;
		inherited::operator>>(key);
		// std::cout << typeid(t).name() << " - " << key << std::endl;
		auto founded = nodes_.find(key);
		if (founded == nodes_.end()) {
			type_t* obj;
			inherited::operator>>(obj);
			nodes_.emplace(key, (void*)&t);
			t = odb::detail::node_builder::template create_new<T>(obj);
		} else {
			t = *reinterpret_cast<T*>(founded->second);
		}
		return (*this);
	}
	// basic node serialization
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::basic_serialization, T>,
		binary_iarchive_special>::type&
	serialize_node(T & t) {
		return inherited::operator>>(t);
	}

	// special list serialization 
	template<class T> typename std::enable_if<
	std::is_base_of_v<odb::special_serialization, T>,
		binary_iarchive_special>::type&
	serialize_list(T& t) {
		operator>>(t.n_);
		return (*this);
	}
	// basic node serialization
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::basic_serialization, T>,
		binary_iarchive_special>::type&
	serialize_list(T & t) {
		return inherited::operator>>(t);
	}
};

class binary_oarchive_special 
	: public binary_oarchive_impl<binary_oarchive_special, std::ostream::char_type, std::ostream::traits_type>
	, public binary_special {
public:
	using inherited = binary_oarchive_impl<
		binary_oarchive_special, std::ostream::char_type, std::ostream::traits_type>;

	binary_oarchive_special(std::ostream& os, unsigned int flags = 0) :
		inherited(os, flags) {}
	binary_oarchive_special(std::streambuf& bsb, unsigned int flags = 0) :
		inherited(bsb, flags) {}
	
	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_node_abstract, T>,
		binary_oarchive_special>::type&
	operator&(const T& t) {
		return serialize_node(t);
	}
	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_list_abstract, T>,
		binary_oarchive_special>::type&
		operator&(const T& t) {
		return serialize_list(t);
	}
	template<class T>
	typename std::enable_if<
		!std::disjunction_v<
			std::is_base_of<odb::detail::basic_node_abstract, T>,
			std::is_base_of<odb::detail::basic_list_abstract, T>
		>, binary_oarchive_special>::type&
	operator&(const T & t) {
		return inherited::operator<<(t);
	}

	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_node_abstract, T>,
		binary_oarchive_special>::type&
		operator<<(const T& t) {
		return serialize_node(t);
	}
	template<class T>
	typename std::enable_if<
		std::is_base_of_v<odb::detail::basic_list_abstract, T>,
		binary_oarchive_special>::type&
		operator<<(const T& t) {
		return serialize_list(t);
	}
	// not a node
	template<class T>
	typename std::enable_if<
		!std::disjunction_v<
		std::is_base_of<odb::detail::basic_node_abstract, T>,
		std::is_base_of<odb::detail::basic_list_abstract, T>
		>, binary_oarchive_special>::type&
		operator<<(const T & t) {
		return inherited::operator<<(t);
	}
	template<class T>
	void serialize_default(const T& t) {
		inherited::operator<<(t);
	}
protected:
	std::unordered_set<uint64_t> nodes_;
	// special node serialization
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::special_serialization, T>,
		binary_oarchive_special>::type& 
	serialize_node(const T & t) {
		auto obj = t.get();
		auto key = reinterpret_cast<uint64_t>(obj);
		// std::cout << typeid(t).name() << " - " << key << std::endl;
		inherited::operator<<(key);
		if (nodes_.find(key) == nodes_.end()) {
			nodes_.emplace(reinterpret_cast<uint64_t>(t.get()));
			operator<<(t.get());
		}
		return (*this);
	}
	// basic node serialization
	template<class T> typename std::enable_if<
	std::is_base_of_v<odb::basic_serialization, T>,
		binary_oarchive_special>::type& 
	serialize_node(const T& t) {
		return inherited::operator<<(t);
	}
	// special list serialization 
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::special_serialization, T>,
		binary_oarchive_special>::type&
	serialize_list(const T & t) {
		serialize_node(t.n_);
		return (*this);
	}
	// basic list serialization
	template<class T> typename std::enable_if<
		std::is_base_of_v<odb::basic_serialization, T>,
		binary_oarchive_special>::type&
	serialize_list(const T& t) {
		return inherited::operator<<(t);
	}
};

} // namespace archive
} // namespace boost

// required by export
BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::archive::binary_iarchive_special)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::archive::binary_iarchive_special)
BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::archive::binary_oarchive_special)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::archive::binary_oarchive_special)

template<typename Archive>
inline constexpr bool is_special_archive_v = std::is_base_of_v<boost::archive::binary_special, Archive>;

template<typename Archive, typename T>
std::enable_if_t<!std::is_base_of_v<boost::archive::binary_special, Archive>> 
serialize_if_not_special(T& t, Archive& ar, T) { ar & t;}

template<typename Archive, typename T>
std::enable_if_t<std::is_base_of_v<boost::archive::binary_special, Archive>>
serialize_if_not_special(T& t, Archive& ar, T value) {
	if constexpr(Archive::is_loading::value) {
		t = value;
	}
}

template<typename Archive, typename T>
std::enable_if_t<!std::is_base_of_v<boost::archive::binary_special, Archive>>
serialize_default_override(T& t, Archive& ar) { ar & t; }

template<typename Archive, typename T>
std::enable_if_t<std::is_base_of_v<boost::archive::binary_special, Archive>>
serialize_default_override(T& t, Archive& ar) {
	ar.serialize_default(t);
}

