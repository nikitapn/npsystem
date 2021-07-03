#pragma once

namespace gdiclr {
	constexpr auto red = RGB(255, 0, 0);
	constexpr auto green = RGB(0, 255, 0);
	constexpr auto blue = RGB(0, 0, 255);
	constexpr auto gray = RGB(230, 230, 230);
	constexpr auto white = RGB(255, 255, 255);
	constexpr auto black = RGB(0, 0, 0);
};

struct GDIGlobal {
	// colors
	COLORREF clrStatusBar;
	COLORREF clrButton;
	// brushes
	CBrush brBackground;
	CBrush brGreen;
	CBrush brRed;
	CBrush brFocusedTab;
	CBrush brActiveTab;
	CBrush brActiveTabNF;
	CBrush brFocusedTabButton;
	CBrush brDialog;
	CBrush brOnlineTab;
	
	// fonts
	CFont fntMonospace;
	CFont fntTreeView;
	CFont fntVerticalRight;
	CFont fntVerticalLeft;

	GDIGlobal();
};

extern GDIGlobal gdiGlobal;

