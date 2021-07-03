#pragma once

#include "peripheral.h"
#include "atmega8_def.h"
#include "atmega16_def.h"
#include <functional>
#include "../include/sim/medium.h"
#include "event_list.h"

class AtmegaUART : public AvrPeripheral {
	using MC = Atmega8;
	int dev_addr_;
public:
	void Setup(sram_t& sram, uint64_t tick_duration, EventList& ev, uint8_t dev_addr) {
		AvrPeripheral::Setup(sram, tick_duration, ev);
		dev_addr_ = dev_addr;
	}
	int Execute(uint64_t time_gap) {
		auto ucsrb = reg(MC::UCSRB);
		// auto ucsra = reg(MC::UCSRA);

		if ((ucsrb & (1 << MC::TXEN)) && ev_->tx_in_progress &&
			mstate_->is_tx_complete()) {
			ev_->tx_in_progress = false;
			reg(MC::UCSRA) &= ~(1 << MC::TXC);
			return MC::USART_Tx_Complete;
		}
		if (ucsrb & (1 << MC::RXEN) && !ev_->tx_in_progress) {
			auto byte = mstate_->uart_get_sended_byte();
			if (byte) {
				reg(MC::UDR) = byte.value();
				return MC::USART_Rx_Complete;
			}
		}
		return -1;
	}
};

template<typename MC>
class AtmegaExtInterrupt0 : public AvrPeripheral {
public:
	int Execute(uint64_t time_gap) {
		if (mstate_->is_int_0_changed()) {
			auto pind = reg(MC::PIND);
			pind &= ~(1 << MC::PD2);
			pind |= (mstate_->get_int0_state() << MC::PD2);
			reg(MC::PIND) = pind; //atomicly
		}

		auto gicr = reg(MC::GICR);
		if (!(gicr & (1 << MC::INT0))) return -1;

		//auto gifr = reg(MC::GIFR);
		auto mcucr = reg(MC::MCUCR);

		if ((mcucr & (1 << MC::ISC00)) && (mcucr & (1 << MC::ISC01))) { // rising edge
			if (mstate_->get_int0_edge() == MediumState::rising_edge) {
				// std::cout << "rising_edge" << std::endl;
				return MC::External_Interrupt_Request_0;
			}
		} else if (mcucr & (1 << MC::ISC01)) { // falling edge
			if (mstate_->get_int0_edge() == MediumState::falling_edge) {
				// std::cout << "falling edge" << std::endl;
				return MC::External_Interrupt_Request_0;
			}
		} else if (mcucr & (1 << MC::ISC00)) { // any edge
			if (mstate_->is_int_0_changed()) {
				// std::cout << "any edge" << std::endl;
				return MC::External_Interrupt_Request_0;
			}
		} else {
			if (mstate_->get_int0_state() == MediumState::logic_zero) {
				return MC::External_Interrupt_Request_0;
			}
		}
		return -1;
	}
};

using Atmega8ExtInterrupt0 = AtmegaExtInterrupt0<Atmega8>;
using Atmega16ExtInterrupt0 = AtmegaExtInterrupt0<Atmega16>;

/*
class AtmegaExtInterrupt1 : public AvrPeripheral {
	using MC = Atmega8;
public:
	int Execute(uint64_t time_gap) {
		auto gicr = reg(MC::GICR);
		if (!(gicr & (1 << MC::INT1))) return -1;
		
		auto gifr = reg(MC::GIFR);
		auto mcucr = reg(MC::MCUCR);

		if ((mcucr & (1 << MC::ISC10)) && (mcucr & (1 << MC::ISC11))) { // rising edge
			if (mstate_->get_int1_edge() == MediumState::rising_edge) {
				// std::cout << "rising_edge" << std::endl;
				return MC::External_Interrupt_Request_1;
			}
		} else if (mcucr & (1 << MC::ISC11)) { // falling edge
			if (mstate_->get_int1_edge() == MediumState::falling_edge) {
				// std::cout << "falling edge" << std::endl;
				return MC::External_Interrupt_Request_1;
			}
		} else if (mcucr & (1 << MC::ISC10)) { // any edge
			if (mstate_->is_int_1_changed()) {
				// std::cout << "any edge" << std::endl;
				return MC::External_Interrupt_Request_1;
			}
		} else {
			if (mstate_->get_int1_state() == MediumState::logic_zero) {
				return MC::External_Interrupt_Request_1;
			}
		}
		return -1;
	}
};
*/
