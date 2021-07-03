#pragma once

// CPropertyGridCombo frame

#define WM_PG_COMBOSELCHANGED (WM_USER + 487)

class CPropertyGridCombo : public CWindowImpl<CPropertyGridCombo, CControlWindow>
{
	using base = CWindowImpl<CPropertyGridCombo, CControlWindow>;
public:
	DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW, -1)

	CPropertyGridCombo();
	virtual ~CPropertyGridCombo();

	HWND Create(CWindow parent, const CRect& rc, DWORD dwStyle, int nId);
	void SetFont(CFontHandle font, BOOL bRedraw = TRUE) noexcept;
	void SetColors(COLORREF clrBack, COLORREF clrText, COLORREF clrFocus, COLORREF clrHilite) noexcept;

	void AddString(const std::wstring& strItem) noexcept;
	void SetCurSel(int nItem) noexcept;
	void Clear() noexcept;
protected:
	std::vector<std::wstring> m_Items;
	int m_nSelected;

	CFontHandle m_font;
	int m_line_height;

	bool m_bTracking;

	COLORREF m_clrBack;
	COLORREF m_clrText;
	COLORREF m_clrFocus;
	COLORREF m_clrHilite;
public:
	BEGIN_MSG_MAP(CMyPropertyGridCtrl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnKeyDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()
protected:
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

inline void CPropertyGridCombo::AddString(const std::wstring& strItem) noexcept {
	m_Items.push_back(strItem);
}

inline void CPropertyGridCombo::SetCurSel(int nItem) noexcept {
	m_nSelected = nItem;
}

inline void CPropertyGridCombo::SetFont(CFontHandle font, BOOL bRedraw) noexcept {
	m_font = font;
}

inline void CPropertyGridCombo::SetColors(COLORREF clrBack, COLORREF clrText, COLORREF clrFocus, COLORREF clrHilite) noexcept {
	m_clrBack = clrBack;
	m_clrText = clrText;
	m_clrFocus = clrFocus;
	m_clrHilite = clrHilite;
}
