// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <memory>
#include "cpu.h"

#include <functional>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/serialization/export.hpp>

#include "hexparserflash.h"
#include <npdb/db_serialization.h>
#include "../include/sim/medium.h"

class AvrMicrocontroller 
	: public odb::common_interface<AvrMicrocontroller>
	, public IController
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) { }
public:
	// throws exception file not found
	virtual void LoadFlash(std::string hex_file) = 0;
	virtual AVRCore* GetCore() = 0;
	virtual void PrintIOSpace() = 0;
	virtual sram_t& GetSram() = 0;
	virtual Flash& GetFlash() = 0;
	virtual double GetTime() const = 0;
	virtual uint64_t GetFrequency() const noexcept = 0;
};

template<class T, class Model>
class Microcontroller_Base : public AvrMicrocontroller
{
public:
	using _interface = AvrMicrocontroller;
protected:
	using MC = Model;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void load(Archive& ar, const unsigned int version) {
		sram_.resize(MC::RAMEND + 1);
		memset(&sram_[0], 0x00, MC::RAMEND + 1);
		core = std::make_unique<AVRCore>(MC::CHIP, flash_, sram_, eeprom_, 
			(uint16_t*)(&sram_[0] + MC::SPL), &sram_[0] + MC::SREG, &sram_[0] + MC::UCSRA, MC::PAGE_SIZE, dev_addr_);
	}
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const {}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<AvrMicrocontroller>(*this);
		ar & frequency_;
		ar & tick_duration_s_;
		ar & tick_duration_ns_;
		ar & hex_file_;
		ar & flash_;
		ar & eeprom_;
		ar & dev_addr_;
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	uint64_t frequency_;
	double tick_duration_s_;
	uint64_t tick_duration_ns_;
	std::string hex_file_;
	
	Flash flash_;
	sram_t sram_;
	eeprom_t eeprom_;
	std::unique_ptr<AVRCore> core;
	uint8_t dev_addr_;

	double time_internal_ = 0.0;
	double last_instruction_duration_ = 0.0;
public:
	Microcontroller_Base() = default;
	Microcontroller_Base(uint64_t frequency, uint8_t dev_addr) noexcept
		: frequency_(frequency)
		, tick_duration_s_(1.0 / (double)frequency)
		, tick_duration_ns_(1'000'000'000 / frequency)
		, dev_addr_(dev_addr)
	{

		flash_.init(MC::FLASH_SIZE / 2);

		sram_.resize(MC::RAMEND + 1);
		memset(&sram_[0], 0x00, MC::RAMEND + 1);

		eeprom_.resize(MC::EEPROM_SIZE);
		memset(&eeprom_[0], 0x00, MC::EEPROM_SIZE); // to do

		core = std::make_unique<AVRCore>(MC::CHIP, flash_, sram_, eeprom_,
			(uint16_t*)(&sram_[0] + MC::SPL), &sram_[0] + MC::SREG, &sram_[0] + MC::UCSRA, MC::PAGE_SIZE, dev_addr_);
	}
	virtual sram_t& GetSram() { return sram_; }
	virtual Flash& GetFlash() { return flash_; }
	virtual void LoadFlash(std::string hex_file) override
	{
		hex_file_ = hex_file;
		
		HexParserFlash hp(hex_file);
		hp.Read(flash_);
		
		core->Decode(0, MC::FLASH_SIZE / 2);
	}
	virtual uint64_t GetFrequency() const noexcept final {
		return frequency_;
	}
	virtual AVRCore* GetCore() override
	{
		return core.get();
	}
	virtual double GetTime() const { return time_internal_; };
	virtual void PrintIOSpace() override
	{
	/*	int k = 1;
		for (int i = 0; *MC::io_registers[i]; i++)
		{
			printf("%-6s = 0x%.2x  ", MC::io_registers[i], sram_[i + MC::IO_OFFSET]);
			if (!(k++ % 8))
			{
				printf("\n");
				k = 1;
			}
		}
	*/
	}
	virtual void SetMedium(Medium* medium) noexcept final {
		core->SetMediumState(medium->GetState());
		boost::fusion::for_each((*static_cast<T*>(this)).peripheral_, [&](auto& x) {
			x.SetMedium(medium);
		});
	}
	virtual void ExecutePeripheral(uint64_t time_gap) noexcept final {
		boost::fusion::for_each((*static_cast<T*>(this)).peripheral_, [&](auto& x) {
			int result = x.Execute(time_gap);
			if (result != -1) core->Interrupt(result);
		});
	}
	virtual double ExecuteCore() noexcept final {
		last_instruction_duration_ = (double)core->Execute() * tick_duration_s_;
		time_internal_ += last_instruction_duration_;
		return last_instruction_duration_;
	}
};

template<class Model> 
class Microcontroller;

