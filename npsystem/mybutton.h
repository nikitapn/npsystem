// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

class CMyButton : public CWindowImpl<CMyButton, CButton>
{
	static constexpr COLORREF color = RGB(204, 204, 204);
	static constexpr COLORREF color_text = RGB(0, 0, 0);
	static constexpr COLORREF color_focused = RGB(41, 143, 204);
	static constexpr COLORREF color_disabled = RGB(230, 230, 230);
	static constexpr COLORREF color_disabled_text = RGB(200, 200, 200);
public:
	DECLARE_WND_SUPERCLASS(NULL, CButton::GetWndClassName())

	CMyButton() {
		m_bHlt = m_bTrack = false;
		m_brush.CreateSolidBrush(color);
		m_brush_focused.CreateSolidBrush(color_focused);
		m_brush_disabled.CreateSolidBrush(color_disabled);
	}
	BEGIN_MSG_MAP(CMyButton)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
		MESSAGE_HANDLER(OCM_DRAWITEM, DrawItem)
	END_MSG_MAP()
protected:
	bool m_bHlt;
	bool m_bTrack;
	CBrush m_brush;
	CBrush m_brush_focused;
	CBrush m_brush_disabled;
protected:
	void TrackMouse() {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		tme.hwndTrack = m_hWnd;

		TrackMouseEvent(&tme);
	}
	LRESULT DrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		UINT uStyle = DFCS_BUTTONPUSH;

		// This code only works with buttons.
		ATLASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

		// If drawing selected, add the pushed style to DrawFrameControl.
		//	if (lpDrawItemStruct->itemState & ODS_SELECTED) 
		//		uStyle |= DFCS_PUSHED;

		// Draw the button frame.
		//	::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem,
		//		DFC_BUTTON, uStyle);

		COLORREF text_color;
		if (lpDrawItemStruct->itemState & ODS_DISABLED) {
			text_color = color_disabled_text;
			::SetBkColor(lpDrawItemStruct->hDC, color_disabled);
			::FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, m_brush_disabled);
		} else {
			text_color = color_text;
			if (!m_bHlt) {
				::SetBkColor(lpDrawItemStruct->hDC, color);
				::FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, m_brush);
			} else {
				::SetBkColor(lpDrawItemStruct->hDC, color_focused);
				::FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, m_brush_focused);
			}
		}

		// Get the button's text.
		wchar_t text[256];
		auto len = GetWindowText(text, 256);
		// Draw the button text using the text color red.
		::SetTextColor(lpDrawItemStruct->hDC, text_color);
		::DrawText(lpDrawItemStruct->hDC, text, len,
			&lpDrawItemStruct->rcItem, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

		return NULL;
	}
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		if (!m_bTrack) {
			TrackMouse();
			m_bTrack = true;
		}
		bHandled = FALSE;
		return NULL;
	}
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		m_bHlt = false;
		m_bTrack = false;
		Invalidate();
		bHandled = TRUE;
		return NULL;
	}
	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		// TODO: Add your message handler code here and/or call default
		m_bHlt = true;
		Invalidate();
		bHandled = FALSE;
		return NULL;
	}
};
