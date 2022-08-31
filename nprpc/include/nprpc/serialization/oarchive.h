#pragma once

#include <nprpc/serialization/serialization.h>
#include <ostream>

namespace nprpc::serialization {
class binary_oarchive : public basic_oarchive<binary_oarchive> {
public:	
	template<typename U>
	void save_fundamental(const U obj) {
		os_.write(reinterpret_cast<const char*>(&obj), sizeof(U));
	}
	
	void save_byte_sequence(const char* data, unsigned int size) {
		os_.write(data, size);
	}

	binary_oarchive(std::ostream& os)
		: basic_oarchive<binary_oarchive>(os) {}
};

class text_oarchive : public basic_oarchive<text_oarchive> {
public:
	template<typename U>
	void save_fundamental(const U obj) {
		os_ << obj;
	}
	
	void save_byte_sequence(const char* data, unsigned int size) {
		os_.write(data, size);
	}

	text_oarchive(std::ostream& os)
		: basic_oarchive<text_oarchive>(os) {}
};


class json_oarchive : public basic_oarchive<json_oarchive> {
	int ident_ = 0;
	bool first_in_object = true;
public:
	void bracket_open() {
		first_in_object = true;
		os_ << "{";
		++ident_;
	}

	void bracket_close() {
		os_ << '\n';
		--ident_;
		for (int i = 0; i < ident_; ++i) os_ << '\t';
		os_ << "}";	
	}

	template<size_t N, typename U>
	void save_nvp(const nvp<N, U>& n) {
		if (!first_in_object) os_ << ',';
		os_ << '\n';
		
		for (int i = 0; i < ident_; ++i) os_ << '\t';
		
		os_ << '\"' << n.name.cv() << "\": ";
		(*this) << n.obj;

		first_in_object = false;
	}

	template<typename U>
	void save_fundamental(const U& obj) {
		os_ << obj;
	}

	void save_byte_sequence(const char* data, unsigned int size) {
		// os_.write(data, size);
	}

	void save_bool(const bool b) {
		os_ << (b ? "true" : "false");
	}

	json_oarchive(std::ostream& os)
		: basic_oarchive<json_oarchive>(os) {}

};

} // namespace nprpc::serialization
