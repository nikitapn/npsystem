// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

struct RibbonState {
	bool cursor_selected = true;
	WORD block_selected = 0;

	void ResetBlockGallery();
};

extern RibbonState g_ribbonState;