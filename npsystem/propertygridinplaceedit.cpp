#include "stdafx.h"
#include "ControlWindow.h"
#include "PropertyGridInPlaceEdit.h"

HWND CPropertyGridInPlaceEdit::Create(CWindow parent, const CRect& rect, const std::string& sInitText, DWORD dwStyle, UINT nID) {
	ATLASSERT(parent != NULL);
	ATLASSUME(m_hWnd == NULL);

	m_sInitText = sInitText;
	m_Rect = rect;

	HWND hWnd = base::Create(parent, m_Rect, L"", dwStyle, 0, nID);

	ATLASSERT(hWnd != NULL);

	SetFont(parent.GetFont());
	SetWindowText(wide(m_sInitText).c_str());
	SetFocus();

	SetSel(0, -1);
	SetSel(-1, 0);

	m_bCancel = FALSE;

	return hWnd;
}

void CPropertyGridInPlaceEdit::SetColors(COLORREF clrBack, COLORREF clrText) {
	m_clrBack = clrBack;
	m_clrText = clrText;
	m_Brush.DeleteObject();
	m_Brush.CreateSolidBrush(m_clrBack);
}

////////////////////////////////////////////////////////////////////////////
// CPropertyGridInPlaceEdit message handlers

// If an arrow key (or associated) is pressed, then exit if
//  a) The Ctrl key was down, or
//  b) m_bExitOnArrows == TRUE

LRESULT CPropertyGridInPlaceEdit::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if ((wParam == VK_PRIOR || wParam == VK_NEXT ||
		wParam == VK_DOWN || wParam == VK_UP ||
		wParam == VK_RIGHT || wParam == VK_LEFT) &&
		(m_bExitOnArrows || GetKeyState(VK_CONTROL) < 0)) {
		GetParent().SetFocus();
		return 0;
	}

	bHandled = FALSE;

	return 0;
}

// As soon as this edit loses focus, kill it.
LRESULT CPropertyGridInPlaceEdit::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (m_bCancel == FALSE)
		EndEdit();
	return 0;
}

LRESULT CPropertyGridInPlaceEdit::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	TCHAR nChar = (TCHAR)wParam;

	if (nChar == VK_TAB || nChar == VK_RETURN) {
		GetParent().SetFocus();    // This will destroy this window
		return 0;
	}
	if (nChar == VK_ESCAPE) {
		CancelEdit();
		return 0;
	}

	bHandled = FALSE;

	return 0;
}

LRESULT CPropertyGridInPlaceEdit::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return DLGC_WANTALLKEYS;
}

////////////////////////////////////////////////////////////////////////////
// CPropertyGridInPlaceEdit overrides

/*
BOOL CPropertyGridInPlaceEdit::PreTranslateMessage(MSG* pMsg) {
	// Catch the Alt key so we don't choke if focus is going to an owner drawn button
	if (pMsg->message == WM_SYSCHAR)
		return TRUE;
	return CEdit::PreTranslateMessage(pMsg);
}
*/

////////////////////////////////////////////////////////////////////////////
// CPropertyGridInPlaceEdit implementation

void CPropertyGridInPlaceEdit::CancelEdit() {
	m_bCancel = TRUE;
	// restore previous text
	if (IsWindow())
		DestroyWindow();
}

void CPropertyGridInPlaceEdit::EndEdit() {
	// EFW - BUG FIX - Clicking on a grid scroll bar in a derived class
	// that validates input can cause this to get called multiple times
	// causing assertions because the edit control goes away the first time.
	static BOOL bAlreadyEnding = FALSE;

	if (bAlreadyEnding)
		return;

	bAlreadyEnding = TRUE;
	m_text = GetWindowTextToStdStringW(m_hWnd);

	CWindow parent = GetParent();
	parent.SendMessage(WM_PG_ENDLABELEDIT, (WPARAM)&m_text, NULL);

	bAlreadyEnding = FALSE;
}

LRESULT CPropertyGridInPlaceEdit::OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle dc((HDC)lParam);
	dc.SetTextColor(m_clrText);
	dc.SetBkColor(m_clrBack);
	return (LRESULT)m_Brush.m_hBrush;
}