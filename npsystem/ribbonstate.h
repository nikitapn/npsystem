#pragma once

struct RibbonState {
	bool cursor_selected = true;
	WORD block_selected = 0;

	void ResetBlockGallery();
};

extern RibbonState g_ribbonState;