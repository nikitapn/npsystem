// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef CPU_H_
#define CPU_H_

#include "mctypes.h"
#include <sim/import_export.h>
#include <string>
#include <sstream>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/circular_buffer.hpp>
#include "common.h"
#include "../include/sim/special.h"
#include "event_list.h"

#ifdef _MSC_VER
#	include "windows.h"
#endif // _MSC_VER


class invalid_lpm_source : public std::runtime_error
{
public:
	invalid_lpm_source() : 
		std::runtime_error("invalid lpm source") {}
};

class unknown_insruction : public std::runtime_error
{
public:
	unknown_insruction(uint16_t instruction) :
		std::runtime_error("unknown instruction") {
		instruction_ = instruction;
	}
	uint16_t instruction_;
};

class AVRCore
{
public:
	SIM_IMPORT_EXPORT AVRCore(MICROCONTROLLER mc, Flash& flash, sram_t& sram,  eeprom_t& eeprom, 
		uint16_t* sp_ptr, uint8_t* sreg_ptr, uint8_t* ucsra_ptr, int page_size, uint8_t dev_addr);
	SIM_IMPORT_EXPORT int Execute();
	SIM_IMPORT_EXPORT void PrintRegisterFile() const;
	SIM_IMPORT_EXPORT void PrintSRAM() const;
	SIM_IMPORT_EXPORT void PrintCurrentInstruction() const;
	SIM_IMPORT_EXPORT void Decode(uint16_t begin_w, uint16_t end_w);
#ifdef _MSC_VER
	SIM_IMPORT_EXPORT int32_t Draw(HDC hdc) const;
#endif
	SIM_IMPORT_EXPORT void PrintLastInstructions();
	size_t flash_size() const { 
		return flash_.size(); 
	}
	void Interrupt(uint16_t address) {
		interrupts_.push(address); 
	}
	void SetMediumState(MediumState* mstate) noexcept {
		mstate_ = mstate;
	}
	EventList ev_list_;
protected:
	int DecodeInstruction(uint16_t& instruction) const;
	uint32_t do_spm();
	uint32_t write_eeprom();
	uint32_t read_eeprom();
	std::unique_ptr<uint16_t> spm_buf_;
	MICROCONTROLLER mc_;
	uint32_t cycle_count_;
	Flash& flash_;
	sram_t& sram_;
	eeprom_t& eeprom_;
	uint8_t* r_file_;
	uint8_t* io_space_;
	uint8_t* sreg_ptr_;
	uint8_t* ucsra_ptr_;
	uint16_t* sp_ptr_;
	uint8_t dev_addr_;
	int pc_;
	void normalize_pc();
	const int page_size_;
	boost::circular_buffer<uint16_t> last_pc_;
	using int_queue_t = boost::lockfree::spsc_queue<uint16_t, boost::lockfree::capacity<16>>;
	int_queue_t interrupts_;
	MediumState* mstate_ = nullptr;
};

#endif // CPU_H_