// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

namespace PRB {
	enum Section {
		ID_SECTION_GENERAL,
		ID_SECTION_POSITION,
		ID_SECTION_BLOCK_SPECIAL,
	};
	enum Property {
		ID_PR_ALG_SCAN_PERIOD = 1,

		ID_PR_ID = 100,
		ID_PR_NAME,
		ID_PR_POSITION_X,
		ID_PR_POSITION_Y,
		ID_PR_DEFAULT_VALUE,
	};
}