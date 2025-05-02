// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "avrmc.h"
#include "peripheral_atmega8.h"
#include "peripheral_uart.h"

template<>
class Microcontroller<Atmega8> :
	public Microcontroller_Base<Microcontroller<Atmega8>, Atmega8>
{
	using self = Microcontroller<Atmega8>;
	using base = Microcontroller_Base<Microcontroller<Atmega8>, Atmega8>;
public:
	Microcontroller() = default;
	Microcontroller(uint64_t frequency, uint8_t dev_addr) noexcept 
		: Microcontroller_Base(frequency, dev_addr) {
		set_peripheral();
	}
	struct Peripheral {
		Atmega8Timer0 timer0;
		Atmega8Timer2 timer2;
		AtmegaUART uart;
		Atmega8ExtInterrupt0 ext_int_0;
	} peripheral_;
private:
	void set_peripheral() {
		assert(sram_.size() != 0);
		peripheral_.timer0.Setup(sram_, tick_duration_ns_, core->ev_list_);
		peripheral_.timer2.Setup(sram_, tick_duration_ns_, core->ev_list_);
		peripheral_.uart.Setup(sram_, tick_duration_ns_, core->ev_list_, dev_addr_);
		peripheral_.ext_int_0.Setup(sram_, tick_duration_ns_, core->ev_list_);
	}
	friend class boost::serialization::access;
	template<class Archive>
	void load(Archive& /* ar */, const unsigned int /* version */) {
		set_peripheral();
	}
	template<class Archive>
	void save(Archive& /* ar */, const unsigned int /* version */) const {}
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & boost::serialization::base_object<base>(*this);
		boost::serialization::split_member(ar, *this, version);
	}
};

BOOST_FUSION_ADAPT_STRUCT (
	Microcontroller<Atmega8>::Peripheral,
	(Atmega8Timer0, timer0)
	(Atmega8Timer2, timer2)
	(AtmegaUART, uart)
	(Atmega8ExtInterrupt0, ext_int_0)
)
