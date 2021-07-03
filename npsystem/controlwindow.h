#pragma once

class CControlWindow : public CWindow
{
public:
	CControlWindow(HWND hwnd = NULL) :
		CWindow(hwnd) {
	}
	virtual ~CControlWindow() {
	}
};