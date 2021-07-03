
#define WM_PG_ENDLABELEDIT (WM_USER + 488)

class CPropertyGridInPlaceEdit : public CWindowImpl<CPropertyGridInPlaceEdit, CEditT<CControlWindow>>
{
	using base = CWindowImpl<CPropertyGridInPlaceEdit, CEditT<CControlWindow>>;
public:
	DECLARE_WND_SUPERCLASS(NULL, CEditT<CControlWindow>::GetWndClassName())
	// Construction
	CPropertyGridInPlaceEdit() {
		m_bExitOnArrows = FALSE;

		m_clrBack = GetSysColor(COLOR_WINDOW);
		m_clrText = GetSysColor(COLOR_WINDOWTEXT);
		m_Brush.CreateSolidBrush(m_clrBack);
	}
	virtual ~CPropertyGridInPlaceEdit() {
	}
	HWND Create(CWindow parent, const CRect& rect, const std::string& sInitText, DWORD dwStyle, UINT nID);
	void SetColors(COLORREF clrBack, COLORREF clrText);
public:
	void CancelEdit();
	void EndEdit();
protected:
	BEGIN_MSG_MAP(CPropertyGridInPlaceEdit)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColor)
	END_MSG_MAP()

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
private:
	std::string m_sInitText;
	std::string m_text;
	BOOL m_bExitOnArrows;
	CRect m_Rect;
	COLORREF m_clrBack;
	COLORREF m_clrText;
	CBrush m_Brush;
	BOOL m_bCancel;
};