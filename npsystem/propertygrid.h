// got it from codeproject

#pragma once

#include <nplib/utils/variant.h>

#include "ControlWindow.h"
#include "PropertyGridCombo.h"
#include "PropertyGridInPlaceEdit.h"
#include "dockable.h"

// CPropertyGrid

#define WM_PG_ITEMCHANGED		(WM_USER + 486)
#define WM_PG_DATESELCHANGED	(WM_USER + 489)

typedef UINT HSECTION;
typedef UINT HITEM;

class ICustomItem;

class CPropertyGrid : public CWindowImpl<CPropertyGrid, CDockableWindowT<CPropertyGrid, DockIndex::PropertyGrid>>
{
	using CMyVariant = npsys::CMyVariant;
public:
	static const int margin = 2;
	// display mode
	enum EDisplayMode
	{
	  DM_CATEGORIZED = 0,
	  DM_ALPHABETICAL,
	  DM_NOSORT
	};

	// editing
	enum EEditMode
	{
	  EM_CUSTOM = 0,
	  EM_INPLACE,
	  EM_DROPDOWN,
	  EM_MODAL
	};

	enum EItemType
	{
	  IT_CUSTOM = 0,
	  IT_VARIANT,
	  IT_BOOLEAN,
	  IT_TEXT,
	  IT_COMBO,
	  IT_DATE,
	  IT_DATETIME,
	  IT_FILE,
	  IT_FOLDER,
	  IT_COLOR,
	  IT_FONT
	};

public:
	struct ComboData
	{
		int index;
	};
	struct FileDialogData
	{
		std::wstring filter;
	};

	using Variant = boost::variant<CMyVariant, std::string, ComboData, FileDialogData, COLORREF, LOGFONT, ICustomItem*>;

	DECLARE_WND_CLASS(NULL)

	CPropertyGrid();
	virtual ~CPropertyGrid();

	// customization
	bool GetShadeTitles();
	void SetShadeTitles(bool shade_titles);
	bool GetDrawLines();
	void SetDrawLines(bool draw_lines);
	bool GetDrawGutter();
	void SetDrawGutter(bool draw_gutter);
	bool GetFocusDisabled();
	void SetFocusDisabled(bool focus_disabled);
	bool GetBoldModified();
	void SetBoldModified(bool bold_modified);
	bool GetBoldEditables();
	void SetBoldEditables(bool bold_editables);

	// gutter width
	int GetGutterWidth();
	void SetGutterWidth(int gutter_width);

	// custom colors
	void SetTextColor(COLORREF clrText);
	void SetTitleColor(COLORREF clrText);
	void SetBackColor(COLORREF clrBack);
	void SetShadeColor(COLORREF clrShade);
	void SetFocusColor(COLORREF clrFocus);
	void SetHiliteColor(COLORREF clrHilite);
	void SetEditableColor(COLORREF clrEditable);
	void SetDisabledColor(COLORREF clrDisabled);

	// localization
	void SetTrueFalseStrings(std::string strTrue, std::string strFalse);
	void SetOkCancelStrings(std::string strOk, std::string strCancel);
	void SetDateTimeStrings(std::string strDate, std::string strTime);
	void SetUndefinedString(std::string strUndefined);
	void SetEmptyString(std::string strEmpty);

	// add a section
	void AddSection(HSECTION id, std::string title, bool collapsed = false, HSECTION after = -1);
	// add items
	HITEM AddVariantItem(HSECTION section, const std::string& name, const CMyVariant& value, UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddTextItem(HSECTION section, const std::string& name, const std::string& value, UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddComboItem(HSECTION section, const std::string& name, const std::vector<std::string>& values, int cur, UINT_PTR tag = 0, bool editable = true, bool undefined = false, HITEM after = -1);
	HITEM AddCustomItem(HSECTION, const std::string& name, ICustomItem* pItem, UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddFileItem(HSECTION section, const std::string& name, const std::string& value, const std::string& filter = "", UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddFolderItem(HSECTION section, const std::string& name, const std::string& value, const std::string& title = "", UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddColorItem(HSECTION section, const std::string& name, COLORREF value, UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
	HITEM AddFontItem(HSECTION section, const std::string& name, LOGFONT value, UINT_PTR tag = 0, bool editable = true, HITEM after = -1);
//	HITEM AddDateItem(HSECTION section, std::string name, COleDateTime value, std::string format = "", bool editable = true, bool undefined = false, HITEM after = -1);
	//	HITEM AddDateTimeItem(HSECTION section, std::string name, COleDateTime value, std::string format = "", bool editable = true, bool undefined = false, HITEM after = -1);
	// contents

	void ResetContents();
	bool RemoveSection(HSECTION hs);
	bool RemoveItem(HITEM item);
	void ValidateChanges();

	// status
	int GetNumSections();
	int GetSectionSize(HSECTION hs);

	// for custom items
	constexpr int GetTextMargin() const noexcept;
	CFontHandle GetFontNormal() noexcept;
	CFontHandle GetFontBold() noexcept;

	// appearance stuff
	void SetDisplayMode(EDisplayMode display_mode);
	void ExpandAll(bool expand);
	void ExpandSection(HSECTION hs, bool expand);
	bool IsSectionCollapsed(HSECTION hs);

	void SetOwner(CWindow owner) noexcept;
protected:

	class CPropertyItem
	{
		
	public:
	  HITEM m_id;
	  EItemType m_type;
	  std::wstring m_name;
	  bool m_editable;
	  //
	  Variant v;
	  Variant v_old;
	  UINT_PTR tag;
	  //
	  
	  CRect m_rcName;
	  CRect m_rcValue;

	  bool operator==(const HITEM& item) const;
	  bool operator==(const std::wstring& name) const;

	  void ValidateChanges();
	};

	class CSection
	{
	public:
	  HSECTION m_id;
	  std::wstring m_title;
	  bool m_collapsed;
	  std::vector<CPropertyItem> m_items;

	  CRect m_rcSign;
	  CRect m_rcTitle;

	  bool operator==(const HSECTION& section) const;
	};

	std::vector<CSection> m_sections;

	HSECTION m_focused_section;
	HITEM m_focused_item;

	EDisplayMode m_display_mode;

	bool m_shade_titles;
	bool m_draw_lines;
	bool m_draw_gutter;
	bool m_focus_disabled;
	bool m_bold_modified;
	bool m_bold_editables;

	int m_gutter_width;
	bool m_resizing_gutter;
	bool m_user_has_resized_gutter;
	CPoint m_ptLast;

	CFont m_fntNormal;
	CFont m_fntBold;

	int m_line_height;

	CRect m_rect_button;
	CControlWindow* m_control;
	CPropertyGridInPlaceEdit m_edit;
	CPropertyGridCombo m_combo;
	bool m_button_pushed;
	bool m_button_depressed;
	bool m_value_clicked;
	bool m_custom_tracking;

	HSECTION m_section_id;
	HITEM m_item_id;

	std::wstring m_strTrue;
	std::wstring m_strFalse;
	std::wstring m_strOk;
	std::wstring m_strCancel;
	std::wstring m_strDate;
	std::wstring m_strTime;
	std::wstring m_strEmpty;

	COLORREF m_clrText;
	COLORREF m_clrTitle;
	COLORREF m_clrBack;
	COLORREF m_clrShade;
	COLORREF m_clrFocus;
	COLORREF m_clrHilite;
	COLORREF m_clrEditable;
	COLORREF m_clrDisabled;

	CWindow m_owner;
protected:
	// init control
	void InitControl();

	// drawing
	void DrawItem(CDC& dc, int w, int x, int y, std::vector<CPropertyItem>::iterator& it);

	// item management
	CSection* FindSection(HSECTION hs) const;
	CPropertyItem* FindItem(HITEM hi) const;
	HITEM AddItem(HSECTION hs, EItemType type, const std::string& name, const void* value, UINT_PTR tag, bool editable, HITEM after);

	// scrolling stuff
	CScrollBar m_scrollbar;
	bool m_scroll_enabled;
	int GetScrollOffset();
	void RecalcLayout();

	// editing
	EEditMode GetEditMode(const CPropertyItem& item);
	void DeleteEditControl();
	void EditFocusedItem();

	// movement in list
	void MoveForward(HSECTION& focused_section, HITEM& focused_item);

	// keyboard
	void FocusNextItem();
	void FocusPrevItem();

public:
	BEGIN_MSG_MAP(CPropertyGrid)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_PG_COMBOSELCHANGED, OnComboSelChanged)
		MESSAGE_HANDLER(WM_PG_ENDLABELEDIT, OnEditChanged)
		MESSAGE_HANDLER(WM_PG_DATESELCHANGED, OnDateChanged)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnComboSelChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

inline constexpr int CPropertyGrid::GetTextMargin() const noexcept {
	return 2 * margin;
}

inline CFontHandle CPropertyGrid::GetFontNormal() noexcept {
	return m_fntNormal.m_hFont;
}

inline CFontHandle CPropertyGrid::GetFontBold() noexcept {
	return m_fntBold.m_hFont;
}

inline void CPropertyGrid::SetOwner(CWindow owner) noexcept {
	ATLASSERT(owner.m_hWnd);
	m_owner = owner;
}

extern CPropertyGrid* g_pPropertyGrid;