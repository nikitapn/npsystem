// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

struct EventList {
	bool increase_tcnt0 = false;
	bool tcnt0_increased = false;
	bool increase_tcnt2 = false;
	bool tcnt2_increased = false;
	bool tx_in_progress = false;
};