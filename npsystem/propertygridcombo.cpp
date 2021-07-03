#include "stdafx.h"
#include "ControlWindow.h"
#include "PropertyGridCombo.h"

static const int margin = 2;

// CPropertyGridCombo

CPropertyGridCombo::CPropertyGridCombo() {
	m_nSelected = -1;
	m_bTracking = false;

	m_clrBack = GetSysColor(COLOR_WINDOW);
	m_clrText = GetSysColor(COLOR_WINDOWTEXT);
	m_clrFocus = GetSysColor(COLOR_HIGHLIGHT);
	m_clrHilite = GetSysColor(COLOR_HIGHLIGHTTEXT);

	m_Items.reserve(32);
}

CPropertyGridCombo::~CPropertyGridCombo() {
}

//
// content management
//
void CPropertyGridCombo::Clear() noexcept {
	m_Items.clear();
	m_nSelected = -1;
	m_bTracking = false;
}
HWND CPropertyGridCombo::Create(CWindow parent, const CRect& rc, DWORD dwStyle, int nId) {
	CRect rect = rc;
	parent.ClientToScreen(&rect);
	HWND hWnd = base::Create(parent, rect, _T(""), dwStyle, 0, nId);
	ATLASSERT(hWnd != NULL);

	SetWindowLongPtr(GWLP_HWNDPARENT, (LONG_PTR)parent.m_hWnd);

	Clear();

	return m_hWnd;
}

LRESULT CPropertyGridCombo::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	BOOL bShow = (BOOL)wParam;
	if (bShow) {
		// get line height
		CDCHandle dc = GetDC();
		int save = dc.SaveDC();
		dc.SelectFont(m_font.m_hFont ? m_font.m_hFont : GetFont());
		
		CSize sz;
		dc.GetTextExtent(_T("Gg"), 2, &sz);
		m_line_height = sz.cy + 2 * margin;
		dc.RestoreDC(save);
		ReleaseDC(dc);

		// size control
		CRect rc;
		GetWindowRect(&rc);
		SetWindowPos(NULL, 0, 0, rc.Width(), int(m_Items.size())*m_line_height + 2, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
		SetFocus();
	}

	bHandled = FALSE;
	
	return 0;
}

LRESULT CPropertyGridCombo::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	return 0;
}

LRESULT CPropertyGridCombo::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	DestroyWindow();
	return 0;
}

//
// painting
//

LRESULT CPropertyGridCombo::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// check
	if (m_nSelected < 0) 
		m_nSelected = 0;

	if (m_nSelected > static_cast<int>(m_Items.size()) - 1) 
		m_nSelected = static_cast<int>(m_Items.size()) - 1;

	// client rect
	CRect rc;
	GetClientRect(&rc);

	// brush
	CBrush brush;
	brush.CreateSolidBrush(m_clrBack);

	// pen
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, m_clrText);

	// the dc
	CPaintDC dc(m_hWnd);
	auto oldBrush = dc.SelectBrush(brush);
	auto oldPen = dc.SelectPen(pen);
	auto oldFont = dc.SelectFont(m_font);

	// draw
	dc.SelectBrush(brush);
	dc.SelectPen(pen);
	dc.Rectangle(rc);

	// put items
	int i = 0;
	int y = 1;

	dc.SelectFont(m_font);
	dc.SetBkMode(TRANSPARENT);

	for (const auto& item : m_Items) {
		CRect rcItem(0, y, rc.Width(), y + m_line_height);
		rcItem.DeflateRect(1, 0, 1, 0);

		if (i == m_nSelected) {
			dc.DrawFocusRect(rcItem);
			dc.SetTextColor(m_clrHilite);

			CRect rc = rcItem;
			rc.DeflateRect(1, 1);
			dc.FillSolidRect(rc, m_clrFocus);
		} else
		{
			dc.SetTextColor(m_clrText);
		}

		// do it
		rcItem.left += 2 * margin;
		dc.DrawText(item.c_str(), static_cast<int>(item.length()), rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
		y += m_line_height;
		i++;
	}

	// clean up
	dc.SelectFont(oldFont);
	dc.SelectPen(oldPen);
	dc.SelectBrush(oldBrush);

	return 0;
}

//
// mouse interaction
//

LRESULT CPropertyGridCombo::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);

	m_nSelected = point.y / m_line_height;
	m_bTracking = true;
	SetCapture();
	Invalidate();

	return 0;
}

LRESULT CPropertyGridCombo::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);

	if (m_bTracking) {
		m_nSelected = point.y / m_line_height;
		Invalidate();
	}
	return 0;
}

LRESULT CPropertyGridCombo::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (m_bTracking) {
		ReleaseCapture();
		m_bTracking = false;
		Invalidate();
	}
	
	auto parent = GetParent();
	parent.SendMessage(WM_PG_COMBOSELCHANGED, m_nSelected, 0);
	
	return 0;
}

//
// keyboard interaction
//

LRESULT CPropertyGridCombo::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return DLGC_WANTALLKEYS;
}

LRESULT CPropertyGridCombo::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (wParam == VK_LEFT || wParam == VK_UP) {
		m_nSelected = std::max(0, m_nSelected - 1);
		Invalidate();
	} else if (wParam == VK_RIGHT || wParam == VK_DOWN) {
		m_nSelected = std::min(int(m_Items.size()) - 1, m_nSelected + 1);
		Invalidate();
	} else if (wParam == VK_ESCAPE) {
		DestroyWindow();
		return 0;
	} else if (wParam == VK_RETURN || wParam == VK_EXECUTE) {
		GetParent().SendMessage(WM_PG_COMBOSELCHANGED, m_nSelected, 0);
		return 0;
	}
	return 0;
}
