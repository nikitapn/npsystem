#include "stdafx.h"
#include "gdiglobal.h"
#include "constants.h"

GDIGlobal gdiGlobal;

GDIGlobal::GDIGlobal() {
	clrStatusBar = RGB(150, 150, 150);
	clrButton = RGB(0, 0, 0);
		
	brBackground.CreateSolidBrush(RGB(156, 158, 154));
	brActiveTabNF.CreateSolidBrush(RGB(136, 138, 134));
	brGreen.CreateSolidBrush(gdiclr::green);
	brRed.CreateSolidBrush(gdiclr::red);
	brActiveTab.CreateSolidBrush(RGB(150, 80, 220));
	brFocusedTab.CreateSolidBrush(RGB(180, 110, 250));
	brFocusedTabButton.CreateSolidBrush(RGB(255, 0, 0));
	brOnlineTab.CreateSolidBrush(RGB(29, 215, 2));
	brDialog.CreateSolidBrush(RGB(255, 255, 255));

	// Fonts

	LOGFONT lf = { 0 };
	lf.lfHeight = constants::treeview::font_height;
	
	{
		static const wchar_t font_name[] = L"Monospace";
		wcscpy_s(lf.lfFaceName, sizeof(font_name) / 2, font_name);
		fntMonospace.CreateFontIndirect(&lf);
	}

	{
		static const wchar_t font_name[] = L"Segoe UI";
		wcscpy_s(lf.lfFaceName, sizeof(font_name) / 2, font_name);
		fntTreeView.CreateFontIndirect(&lf);
	}

	{
		lf.lfEscapement	= 900;
		static const wchar_t font_name[] = L"Segoe UI";
		wcscpy_s(lf.lfFaceName, sizeof(font_name) / 2, font_name);
		fntVerticalLeft.CreateFontIndirect(&lf);
	}

	{
		lf.lfEscapement	= 2700;
		static const wchar_t font_name[] = L"Segoe UI";
		wcscpy_s(lf.lfFaceName, sizeof(font_name) / 2, font_name);
		fntVerticalRight.CreateFontIndirect(&lf);
	}



}