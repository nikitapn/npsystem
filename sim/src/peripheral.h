// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "mctypes.h"
#include "../include/sim/medium.h"
#include "event_list.h"

class AvrPeripheral {
public:
	void Setup(sram_t& sram, uint64_t tick_duration, EventList& ev) {
		sram_ = &sram[0];
		tick_duration_ = tick_duration;
		timer_time_ = 0;
		ev_ = &ev;
	}
	void SetMedium(Medium* medium) noexcept {
		assert(medium);
		medium_ = medium;
		mstate_ = medium_->GetState();
	}
protected:
	uint8_t* sram_;
	uint64_t timer_time_;
	uint64_t tick_duration_;
	EventList* ev_;
	Medium* medium_ = nullptr;
	MediumState* mstate_ = nullptr;
	
	//
	uint8_t& reg(size_t r) noexcept {
		return sram_[r];
	}
};