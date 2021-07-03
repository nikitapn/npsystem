#pragma once

#include "avrmc.h"
#include "peripheral_uart.h"
#include "peripheral_atmega16.h"

template<>
class Microcontroller<Atmega16> : 
	public Microcontroller_Base<Microcontroller<Atmega16>, Atmega16>
{
	using self = Microcontroller<Atmega16>;
	using base = Microcontroller_Base<Microcontroller<Atmega16>, Atmega16>;

	friend class boost::serialization::access;
	template<class Archive>
	void load(Archive& ar, const unsigned int version) {
		set_peripheral();
	}
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const {}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<base>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	Microcontroller() = default;
	Microcontroller(uint64_t frequency, uint8_t dev_addr) noexcept
		: Microcontroller_Base(frequency, dev_addr) {
		set_peripheral();
	}
	struct Peripheral {
		Atmega16Timer0 timer0;
		Atmega16Timer2 timer2;
		AtmegaUART uart;
		Atmega16ExtInterrupt0 ext_int_0;
	} peripheral_;
private:
	void set_peripheral() {
		peripheral_.timer0.Setup(sram_, tick_duration_ns_, core->ev_list_);
		peripheral_.timer2.Setup(sram_, tick_duration_ns_, core->ev_list_);
		peripheral_.uart.Setup(sram_, tick_duration_ns_, core->ev_list_, dev_addr_);
		peripheral_.ext_int_0.Setup(sram_, tick_duration_ns_, core->ev_list_);
	}
};

BOOST_FUSION_ADAPT_STRUCT
(
	Microcontroller<Atmega16>::Peripheral,
	(Atmega16Timer0, timer0)
	(Atmega16Timer2, timer2)
	(AtmegaUART,uart)
	(Atmega16ExtInterrupt0, ext_int_0)
)