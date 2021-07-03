#pragma once

#include <stdint.h>
#include <vector>

#include <boost/serialization/export.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/vector.hpp>

class Flash {
	friend class boost::serialization::access;
public:
	struct instruction_t {
		uint16_t instruction;
		uint8_t op;
		uint8_t size;
		//
		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version) {
			ar & instruction;
			ar & op;
			ar & size;
		}
	};
	void init(size_t size_w) {
		data_.resize(size_w);
		for (auto& i : data_) {
			i.op = 0;
			i.size = 0;
			i.instruction = 0xFFFF;
		}
	}
	size_t size() const {
		return data_.size();
	}
	instruction_t& at(size_t index) {
		return data_.at(index);
	}
	const instruction_t& at(size_t index) const {
		return data_.at(index);
	}
	instruction_t& operator[](size_t index) {
		return data_[index];
	}
	const instruction_t& operator[](size_t index) const {
		return data_[index];
	}
	void Store(uint16_t addr_b, uint16_t instruction) {
		data_[addr_b / 2].instruction = instruction;
	}
	uint8_t get_byte(uint16_t addr_b) {
		return *(reinterpret_cast<uint8_t*>(&data_[addr_b / 2].instruction) + (addr_b % 2));
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & data_;
	}
protected:
	std::vector<instruction_t> data_;
};

using sram_t = std::vector<uint8_t>;
using eeprom_t = std::vector<uint8_t>;