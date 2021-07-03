#pragma once

#include "peripheral.h"
#include "atmega16_def.h"

class Atmega16Timer0 : public AvrPeripheral
{
	using MC = Atmega16;
public:
	int Execute(uint64_t time_gap)
	{
		timer_time_ += time_gap;
		int divider = 0;
		int enabled = sram_[MC::TCCR0] & 0x07;
		
		if (!enabled)
			return -1;

		switch (enabled)
		{
		case 1:
			divider = 1;
			break;
		case 2:
			divider = 8;
			break;
		case 3:
			divider = 64;
			break;
		case 4:
			divider = 256;
			break;
		case 5:
			divider = 1024;
			break;
		case 6:
			//External clock source on T0 pin. Clock on falling edge.
			assert(false);
			break;
		case 7:
			//External clock source on T0 pin. Clock on rising edge.
			assert(false);
			break;
		}
		if (timer_time_ >= tick_duration_ * divider)
		{
			timer_time_ = 0ULL;
			sram_[MC::TCNT0]++;
			if ((sram_[MC::TCCR0] & (1 << MC::WGM01)) && !(sram_[MC::TCCR0] & (1 << MC::WGM00)))
			{
				if (sram_[MC::TCNT0] == sram_[MC::OCR0]) {
					if (sram_[MC::TIMSK] & (1 << MC::OCIE0)) {
						sram_[MC::TIFR] &= ~(1 << MC::OCF0);
						return MC::TimerCounter0_Compare_Match;
					} else {
						sram_[MC::TIFR] |= (1 << MC::OCF0);
					}
				}
				return -1;
			}
			if (!sram_[MC::TCNT0])
			{
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

class Atmega16Timer2 : public AvrPeripheral
{
	using MC = Atmega16;
public:
	int Execute(uint64_t time_gap)
	{
		timer_time_ += time_gap;
		int divider = 0;
		int enabled = sram_[MC::TCCR2] & 0x07;

		if (!enabled)
			return -1;

		switch (enabled)
		{
		case 1:
			divider = 1;
			break;
		case 2:
			divider = 8;
			break;
		case 3:
			divider = 32;
			break;
		case 4:
			divider = 64;
			break;
		case 5:
			divider = 128;
			break;
		case 6:
			divider = 256;
			break;
		case 7:
			divider = 1024;
			break;
		}

		if (timer_time_ >= tick_duration_ * divider)
		{
			timer_time_ = 0ULL;
			sram_[MC::TCNT2]++;
			if ((sram_[MC::TCCR2] & (1 << MC::WGM21)) && !(sram_[MC::TCCR2] & (1 << MC::WGM20)))
			{
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
			if (!sram_[MC::TCNT2])
			{
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