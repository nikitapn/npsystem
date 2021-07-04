// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "peripheral.h"
#include "atmega8_def.h"

class Atmega8Timer0 : public AvrPeripheral
{
	using MC = Atmega8;
public:
	int Execute(uint64_t time_gap) {
		timer_time_ += time_gap;
		int divider = 0;
		auto tccr0 = sram_[MC::TCCR0];
		int enabled = tccr0 & 0x07;

		if (!enabled) return -1;

		switch (enabled) {
			case 1: divider = 1; break;
			case 2: divider = 8; break;
			case 3: divider = 64; break;
			case 4: divider = 256; break;
			case 5: divider = 1024; break;
			case 6: assert(false); break; //External clock source on T0 pin. Clock on falling edge.
			case 7: assert(false); break; //External clock source on T0 pin. Clock on rising edge.
		}
		if (timer_time_ >= tick_duration_ * divider) {
			ev_->increase_tcnt0 = true;
			timer_time_ = 0ULL;
		}
		if (ev_->tcnt0_increased) {
			ev_->tcnt0_increased = false;
			if (!sram_[MC::TCNT0]) {
				if (sram_[MC::TIMSK] & (1 << MC::TOIE0)) {
					sram_[MC::TIFR] &= ~(1 << MC::TOV0);
					return MC::TimerCounter0_Overflow;
				} else {
					sram_[MC::TIFR] |= (1 << MC::TOV0);
				}
			}
		}
		return -1;
	}
};

class Atmega8Timer2 : public AvrPeripheral
{
	using MC = Atmega8;
public:
	int Execute(uint64_t time_gap) {
		timer_time_ += time_gap;
		int divider = 0;
		auto tccr2 = sram_[MC::TCCR2];
		int enabled = tccr2 & 0x07;

		if (!enabled) return -1;

		switch (enabled) {
			case 1: divider = 1; break;
			case 2: divider = 8; break;
			case 3: divider = 32; break;
			case 4: divider = 64; break;
			case 5: divider = 128; break;
			case 6: divider = 256; break;
			case 7: divider = 1024; break;
		}
		if (timer_time_ >= tick_duration_ * divider) {
			ev_->increase_tcnt2 = true;
			timer_time_ = 0ULL;
		}
		if (ev_->tcnt2_increased) {
			ev_->tcnt2_increased = false;
			if ((tccr2 & (1 << MC::WGM21)) && !(tccr2 & (1 << MC::WGM20))) {
				if (sram_[MC::TCNT2] == sram_[MC::OCR2]) {
					if (sram_[MC::TIMSK] & (1 << MC::OCIE2)) {
						sram_[MC::TIFR] &= ~(1 << MC::OCF2);
						return MC::TimerCounter2_Compare_Match;
					}
					else {
						sram_[MC::TIFR] |= (1 << MC::OCF2);
					}
				}
				return -1;
			}
			if (!sram_[MC::TCNT2]) {
				if (sram_[MC::TIMSK] & (1 << MC::TOIE2)) {
					sram_[MC::TIFR] &= ~(1 << MC::TOV2);
					return MC::TimerCounter2_Overflow;
				}
				else {
					sram_[MC::TIFR] |= (1 << MC::TOV2);
				}
			}
		}
		return -1;
	}
};