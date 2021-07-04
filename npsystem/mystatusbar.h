// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "View.h"
#include "gdiglobal.h"
#include <npsys/corba.h>

class CMyStatusBar : public CWindowImpl<CMyStatusBar, CWindow>
{
	using base = CWindowImpl<CMyStatusBar, CWindow>;
	
	static constexpr int statusbar_height = 24;
	static constexpr int statusbar_bulb_dim = 10;
public:

	DECLARE_WND_CLASS(NULL);

	BEGIN_MSG_MAP(CMyStatusBar)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	HWND Create(HWND hwndParent) {
		ATLASSERT(hwndParent != NULL);
		m_wndParent = hwndParent;
		return base::Create(hwndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	}
	void Update(CView* view) {
		if (view != nullptr) {
		}
		Invalidate();
	}
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 0;
	}
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CPaintDC dc(m_hWnd);
	
		CRect rect;
		GetClientRect(rect);
		dc.FillSolidRect(rect, gdiGlobal.clrStatusBar);

		if (npsys_rpc->is_server_exist()) {
			dc.FillSolidRect(m_bulb, gdiclr::green);
		} else {
			dc.FillSolidRect(m_bulb, gdiclr::red);
		}

		return 0;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	UINT width = LOWORD(lParam), height = HIWORD(lParam);
	//	DBG_PRINT("OnSize cx: %d, cy: %d\n", width, height);
		
		CRect rect;
		m_wndParent.GetClientRect(rect);
		SetWindowPos(NULL, 0, rect.bottom - statusbar_height, rect.right - rect.left, statusbar_height, SWP_SHOWWINDOW);
		
		constexpr auto bulb_y = (statusbar_height - statusbar_bulb_dim) / 2;

		m_bulb = CRect(rect.right - statusbar_bulb_dim - 5, bulb_y, rect.right - 5, bulb_y + statusbar_bulb_dim);

		return 0;
	}


	CWindow m_wndParent;
	CRect m_bulb;
	CRect m_text;
};