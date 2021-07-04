// got it from codeproject

#include "stdafx.h"
#include "CustomItem.h"
#include "PropertyGrid.h"
#include <regex>
//#include "PropertyGridDirectoryPicker.h"
//#include "PropertyGridMonthCalCtrl.h"
//#include "DynDialogEx.h"

#include "Global.h"
#include "GDIGlobal.h"

#define IDC_MONTHCAL 1023



CPropertyGrid::CPropertyGrid() {
	m_item_id = 0;
	m_user_has_resized_gutter = false;
	m_resizing_gutter = false;
	m_button_pushed = false;
	m_button_depressed = false;
	m_value_clicked = false;
	m_custom_tracking = false;
	m_scroll_enabled = false;
	m_draw_lines = true;
	m_shade_titles = true;
	m_draw_gutter = true;
	m_focus_disabled = true;
	m_bold_modified = false;
	m_bold_editables = false;
	m_display_mode = DM_CATEGORIZED;
	m_control = nullptr;

	m_rect_button = CRect(0, 0, 0, 0);
	m_ptLast = CPoint(0, 0);

	m_strTrue = L"True";
	m_strFalse = L"False";
	m_strDate = L"Date";
	m_strTime = L"Time";
	m_strEmpty = L"";

	m_clrBack = GetSysColor(COLOR_WINDOW);
	m_clrShade = GetSysColor(COLOR_3DFACE);
	m_clrText = GetSysColor(COLOR_WINDOWTEXT);
	m_clrTitle = GetSysColor(COLOR_WINDOWTEXT);
	m_clrFocus = GetSysColor(COLOR_HIGHLIGHT);
	m_clrHilite = GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_clrEditable = GetSysColor(COLOR_WINDOWTEXT);
	m_clrDisabled = GetSysColor(COLOR_GRAYTEXT);

	m_focused_section = -1;
	m_focused_item = -1;

	m_state.dock = Dock::Right;
}

CPropertyGrid::~CPropertyGrid() {
}

//
// customization
//

bool CPropertyGrid::GetShadeTitles() {
	return m_shade_titles;
}

void CPropertyGrid::SetShadeTitles(bool shade_titles) {
	m_shade_titles = shade_titles;
	if (m_hWnd)
		Invalidate();
}

bool CPropertyGrid::GetDrawLines() {
	return m_draw_lines;
}

void CPropertyGrid::SetDrawLines(bool draw_lines) {
	m_draw_lines = draw_lines;
	if (m_hWnd)
		Invalidate();
}

bool CPropertyGrid::GetDrawGutter() {
	return m_draw_gutter;
}

void CPropertyGrid::SetDrawGutter(bool draw_gutter) {
	m_draw_gutter = draw_gutter;
	if (m_hWnd)
		Invalidate();
}

bool CPropertyGrid::GetFocusDisabled() {
	return m_focus_disabled;
}

void CPropertyGrid::SetFocusDisabled(bool focus_disabled) {
	m_focus_disabled = focus_disabled;
	if (m_hWnd)
		Invalidate();
}

bool CPropertyGrid::GetBoldModified() {
	return m_bold_modified;
}

void CPropertyGrid::SetBoldModified(bool bold_modified) {
	m_bold_modified = bold_modified;
}

bool CPropertyGrid::GetBoldEditables() {
	return m_bold_editables;
}

void CPropertyGrid::SetBoldEditables(bool bold_editables) {
	m_bold_editables = bold_editables;
}

//
// gutter width
//

int CPropertyGrid::GetGutterWidth() {
	return m_gutter_width;
}

void CPropertyGrid::SetGutterWidth(int gutter_width) {
	m_gutter_width = gutter_width;
	if (m_hWnd)
		Invalidate();
}

//
// custom colors
//

void CPropertyGrid::SetTextColor(COLORREF clrText) {
	if (m_clrText == m_clrEditable)
		m_clrEditable = clrText;
	m_clrText = clrText;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetTitleColor(COLORREF clrTitle) {
	m_clrTitle = clrTitle;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetBackColor(COLORREF clrBack) {
	m_clrBack = clrBack;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetShadeColor(COLORREF clrShade) {
	m_clrShade = clrShade;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetFocusColor(COLORREF clrFocus) {
	m_clrFocus = clrFocus;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetHiliteColor(COLORREF clrHilite) {
	m_clrHilite = clrHilite;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetEditableColor(COLORREF clrEditable) {
	m_clrEditable = clrEditable;
	if (m_hWnd)
		Invalidate();
}

void CPropertyGrid::SetDisabledColor(COLORREF clrDisabled) {
	m_clrDisabled = clrDisabled;
	if (m_hWnd)
		Invalidate();
}

//
// localization
//

void CPropertyGrid::SetTrueFalseStrings(std::string strTrue, std::string strFalse) {
	m_strTrue = wide(strTrue);
	m_strFalse = wide(strFalse);
}

void CPropertyGrid::SetOkCancelStrings(std::string strOk, std::string strCancel) {
	m_strOk = wide(strOk);
	m_strCancel = wide(strCancel);
}

void CPropertyGrid::SetDateTimeStrings(std::string strDate, std::string strTime) {
	m_strDate = wide(strDate);
	m_strTime = wide(strTime);
}

void CPropertyGrid::SetEmptyString(std::string strEmpty) {
	m_strEmpty = wide(strEmpty);
}

//
// appearance
//

void CPropertyGrid::SetDisplayMode(EDisplayMode display_mode) {
	m_display_mode = display_mode;
	RecalcLayout();
}

void CPropertyGrid::ExpandAll(bool expand) {
	for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it)
		it->m_collapsed = !expand;
	RecalcLayout();
}

void CPropertyGrid::ExpandSection(HSECTION hs, bool expand) {
	CSection* pSection = FindSection(hs);
	if (pSection) {
		pSection->m_collapsed = !expand;
		RecalcLayout();
	}
}

bool CPropertyGrid::IsSectionCollapsed(HSECTION hs) {
	CSection* pSection = FindSection(hs);
	if (pSection)
		return pSection->m_collapsed;
	return false;
}

//
// item management
//

bool CPropertyGrid::CPropertyItem::operator==(const HITEM& item) const {
	return m_id == item;
}

bool CPropertyGrid::CPropertyItem::operator==(const std::wstring& name) const {
	return m_name == name;
}

bool CPropertyGrid::CSection::operator==(const HSECTION& section) const {
	return m_id == section;
}

void CPropertyGrid::CPropertyItem::ValidateChanges() {
	// save the values
	v_old = v;
	// callback for custom
	if (m_type == IT_CUSTOM)
		boost::get<ICustomItem*>(v)->ValidateChanges();
}

void CPropertyGrid::AddSection(HSECTION id, std::string title, bool collapsed, HSECTION after) {
	// build it
	CSection section;
	section.m_id = id;
	section.m_title = wide(title);
	section.m_collapsed = collapsed;

	// insert it
	// if after does not exist then it is appended
	std::vector<CSection>::iterator it = find(m_sections.begin(), m_sections.end(), after);
	m_sections.insert(it, section);

	// done
	RecalcLayout();
}

HITEM CPropertyGrid::AddItem(HSECTION hs, EItemType type, const std::string& name, const void* value, UINT_PTR tag, bool editable, HITEM after) {
	// check section exists
	std::vector<CSection>::iterator it = find(m_sections.begin(), m_sections.end(), hs);
	if (it == m_sections.end())
		return -1;
	auto name_w = wide(name);
	// check item does not already exists
	std::vector<CPropertyItem>::iterator it2 = find(it->m_items.begin(), it->m_items.end(), name_w);
	if (it2 != it->m_items.end())
		return -1;

	// build the item
	CPropertyItem item;
	item.m_id = m_item_id++;
	item.m_type = type;
	item.m_name = name_w;
	item.tag = tag;
	item.m_editable = editable;
	
	// assign the value
	if (type == IT_CUSTOM)
		item.v = (ICustomItem*)value;
	else if (type == IT_TEXT)// || type == IT_TEXT || type == IT_FILE || type == IT_FOLDER) 
		item.v = *(const std::string*)value;
	else if (type == IT_COMBO)
		item.v = *(const ComboData*)value;
	else if (type == IT_VARIANT) {
		auto& val = *(const CMyVariant*)value;
		item.v = val;
		if (val.which() == 0)
			item.m_type = IT_BOOLEAN;
	}
	//	else if (type == IT_DATE || type == IT_DATETIME) item.m_dtValue = *(COleDateTime*)pValue;
	else if (type == IT_COLOR)
		item.v = *(const COLORREF*)value;
	else if (type == IT_FONT)
		item.v = *(const LOGFONT*)value;
	//	memcpy(&item.data, pValue, sizeof(LOGFONT));
	else 
		ATLASSERT(FALSE);

	// finish and add
	item.ValidateChanges();
	it->m_items.push_back(item);
	RecalcLayout();
	return item.m_id;
}

HITEM CPropertyGrid::AddVariantItem(HSECTION section, const std::string& name, const CMyVariant& value, UINT_PTR tag, bool editable, HITEM after) {
	return AddItem(section, IT_VARIANT, name, &value, tag, editable, after);
}

HITEM CPropertyGrid::AddTextItem(HSECTION section, const std::string& name, const std::string& value, UINT_PTR tag, bool editable, HITEM after) {
	return AddItem(section, IT_TEXT, name, &value, tag, editable, after);
}

HITEM CPropertyGrid::AddCustomItem(HSECTION section, const std::string& name, ICustomItem* pItem, UINT_PTR tag, bool editable, HITEM after) {
	pItem->m_pGrid = this;
	return AddItem(section, IT_CUSTOM, name, pItem, tag, editable, after);
}



HITEM CPropertyGrid::AddComboItem(HSECTION section, const std::string& name, const std::vector<std::string>& values, int cur, UINT_PTR tag, bool editable, bool undefined, HITEM after) {
//	HITEM it = AddItem(section, IT_COMBO, name, &cur, editable, undefined, after);
//	CPropertyItem* pItem = FindItem(it);
//	if (pItem) pItem->m_options = values;
//	return it;
	return 0;
}

HITEM CPropertyGrid::AddFileItem(HSECTION section, const std::string& name, const std::string& value, const std::string& filter, UINT_PTR tag, bool editable, HITEM after) {
	FileDialogData fdata;
	fdata.filter = wide(filter);
	return AddItem(section, IT_FILE, name, &fdata, tag, editable, after);
}

HITEM CPropertyGrid::AddFolderItem(HSECTION section, const std::string& name, const std::string& value, const std::string& title, UINT_PTR tag, bool editable, HITEM after) {
	//	HITEM it = AddItem(section, IT_FOLDER, name, &value, editable, false, after);
	//	CPropertyItem* pItem = FindItem(it);
	//	if (pItem) pItem->m_options.push_back(title);
	//	return it;
	return 0;
}

HITEM CPropertyGrid::AddColorItem(HSECTION section, const std::string& name, COLORREF value, UINT_PTR tag, bool editable, HITEM after) {
	return AddItem(section, IT_COLOR, name, &value, tag, editable, after);
}

HITEM CPropertyGrid::AddFontItem(HSECTION section, const std::string& name, LOGFONT value, UINT_PTR tag, bool editable, HITEM after) {
	return AddItem(section, IT_FONT, name, &value, tag, editable, after);
}

/*
HITEM CPropertyGrid::AddDateItem(HSECTION section, std::string name, COleDateTime value, std::string format, bool editable, bool undefined, HITEM after) {
	HITEM it = AddItem(section, IT_DATE, name, &value, editable, undefined, after);
	CPropertyItem* pItem = FindItem(it);
	if (pItem) pItem->m_options.push_back(format);
	return it;
}


HITEM CPropertyGrid::AddDateTimeItem(HSECTION section, std::string name, COleDateTime value, std::string format, bool editable, bool undefined, HITEM after) {
	HITEM it = AddItem(section, IT_DATETIME, name, &value, editable, undefined, after);
	CPropertyItem* pItem = FindItem(it);
	if (pItem) pItem->m_options.push_back(format);
	return it;
}
*/



void CPropertyGrid::ResetContents() {
	m_sections.clear();
	m_item_id = 0;
	RecalcLayout();
}

bool CPropertyGrid::RemoveSection(HSECTION hs) {
	std::vector<CSection>::iterator it = find(m_sections.begin(), m_sections.end(), hs);
	if (it == m_sections.end()) return false;
	m_sections.erase(it);
	return true;
}

bool CPropertyGrid::RemoveItem(HITEM item) {
	for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
		std::vector<CPropertyItem>::iterator it2 = find(it->m_items.begin(), it->m_items.end(), item);
		if (it2 != it->m_items.end()) {
			it->m_items.erase(it2);
			return true;
		}
	}
	return false;
}

int CPropertyGrid::GetNumSections() {
	return int(m_sections.size());
}

int CPropertyGrid::GetSectionSize(HSECTION hs) {
	CSection* pSection = FindSection(hs);
	if (pSection) return int(pSection->m_items.size());
	return 0;
}

void CPropertyGrid::ValidateChanges() {
	for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
		for (std::vector<CPropertyItem>::iterator it2 = it->m_items.begin(); it2 != it->m_items.end(); ++it2)
			it2->ValidateChanges();
	}
}

CPropertyGrid::CSection* CPropertyGrid::FindSection(HSECTION hs) const {
	std::vector<CSection>::const_iterator it = find(m_sections.begin(), m_sections.end(), hs);
	if (it == m_sections.end()) 
		return NULL;
	return const_cast<CSection*>(&(*it));
}

CPropertyGrid::CPropertyItem* CPropertyGrid::FindItem(HITEM hi) const {
	for (std::vector<CSection>::const_iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
		std::vector<CPropertyItem>::const_iterator it2 = find(it->m_items.begin(), it->m_items.end(), hi);
		if (it2 != it->m_items.end())
			return const_cast<CPropertyItem*>(&(*it2));
	}
	return NULL;
}

//
// creation and window stuff
//

void CPropertyGrid::InitControl() {
	auto parent = GetParent();

	ATLASSERT(parent.m_hWnd != NULL);
	
	SetOwner(parent);

	// first gutter
	CRect rc;
	GetClientRect(&rc);
	m_gutter_width = rc.Width() / 2;

	// check if already done
	if (m_fntNormal.m_hFont == NULL) {
		// fonts
		LOGFONT lf;
	/*	if (GetParent() && GetParent().GetFont()) {
			CFontHandle font = GetParent().GetFont();
			font.GetLogFont(&lf);
			m_fntNormal.CreateFontIndirect(&lf);
			lf.lfWeight = FW_BOLD;
			m_fntBold.CreateFontIndirect(&lf);
		}
		*/
	//	else
		{
			gdiGlobal.fntTreeView.GetLogFont(lf);
			m_fntNormal.CreateFontIndirect(&lf);
		//	m_fntNormal.CreatePointFont(85, "Verdana");
		//	m_fntNormal.GetLogFont(&lf);
			lf.lfWeight = FW_BOLD;
			m_fntBold.CreateFontIndirect(&lf);
		}
	}

	// get line height
	CDCHandle dc = GetDC();
	CFontHandle oldFont = dc.SelectFont(m_fntNormal);
	
	CSize sz;
	dc.GetTextExtent(L"Gg", 2, &sz);
	m_line_height = sz.cy + 2 * margin;
	
	dc.SelectFont(oldFont);
	ReleaseDC(dc);

	// styles
	ModifyStyle(0, WS_CLIPCHILDREN);

	// try to get some strings
	if (m_strOk.length() == 0) m_strOk = L"OK";
	if (m_strCancel.length() == 0) m_strCancel = L"Cancel";
}

LRESULT CPropertyGrid::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	InitControl();
	bHandled = FALSE;
	return 0;
}


LRESULT CPropertyGrid::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	DeleteEditControl();
	bHandled = FALSE;
	return 0;
}

LRESULT CPropertyGrid::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	
	UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
	
	CRect rect;
	GetClientRect(&rect);

	if (!m_user_has_resized_gutter) {
		m_gutter_width = cx / 2;
	} else
	{
		m_gutter_width = m_gutter_width * cx / rect.Width();
	}

	if (m_scrollbar.m_hWnd) {
		m_scrollbar.MoveWindow(rect.right - GetSystemMetrics(SM_CXVSCROLL), rect.top, GetSystemMetrics(SM_CXVSCROLL), rect.Height());
		RecalcLayout();
	}

	return 0;
}

//
// painting
//

LRESULT CPropertyGrid::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return TRUE;
}

LRESULT CPropertyGrid::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// stuff needed
	const int sign_size = 8;

	// the scrollbar offset
	int top = GetScrollOffset();

	// the rect
	CRect rc_dummy;
	GetClientRect(&rc_dummy);
	if (m_scroll_enabled)
		rc_dummy.right -= GetSystemMetrics(SM_CXVSCROLL);

	// make sure we do not modify this one
	// because we use it to bitblt
	const CRect rc(rc_dummy);

	// stuff for flicker free drawing
	CDC dcMem;
	CBitmap bmpMem;
	CPaintDC dc(m_hWnd);

	// create and configure the memdc
	dcMem.CreateCompatibleDC(dc);
	bmpMem.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	auto oldBitmap = dcMem.SelectBitmap(bmpMem);

	// brush needed
	CBrush brushTitle;
	brushTitle.CreateSolidBrush(m_clrTitle);

	// pen needed
	CPen penShade;
	penShade.CreatePen(PS_SOLID, 1, m_clrShade);
	
	CPen penTitle;
	penTitle.CreatePen(PS_SOLID, 1, m_clrTitle);

	// to make sure we won't leak gdi resources
	auto oldBrush = dcMem.SelectBrush(brushTitle);
	auto oldPen = dcMem.SelectPen(penShade);
	auto oldFont = dcMem.SelectFont(m_fntNormal);

	// needed
	int w = rc.Width();

	// blank
	dcMem.FillSolidRect(rc, m_clrBack);
	dcMem.SetBkMode(TRANSPARENT);

	// empty text
	if (m_sections.empty()) {
		CRect rect = rc;
		rect.top += 10;
		rect.DeflateRect(rect.Width() / 4, 0);
		dcMem.DrawText(m_strEmpty.c_str(), static_cast<int>(m_strEmpty.length()), rect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
	} else
	{
		// needed
		int sign_left = margin;

		// we start here
		int y = -top;

		// alphabetical needs special
		if (m_display_mode == DM_ALPHABETICAL) {
			// put all the items in a vector
			std::vector<std::vector<CPropertyItem>::iterator> lst;
			for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
				for (std::vector<CPropertyItem>::iterator it2 = it->m_items.begin(); it2 != it->m_items.end(); ++it2)
					lst.push_back(it2);
			}

			// sort the vector
			std::sort(lst.begin(), lst.end(), [](const auto& a, const auto& b) {
				return a->m_name < b->m_name;
			});

			// display the items
			for (std::vector<std::vector<CPropertyItem>::iterator>::iterator it2 = lst.begin(); it2 != lst.end(); ++it2) {
				// first reset
				(*it2)->m_rcName.SetRectEmpty();
				(*it2)->m_rcValue.SetRectEmpty();

				// draw if visible
				(*it2)->m_rcName = CRect(0, y, w, y + m_line_height);
				CRect rcInter = (*it2)->m_rcName;
				rcInter.IntersectRect(rc, rcInter);
				if (!rcInter.IsRectEmpty())
					DrawItem(dcMem, w, sign_left + sign_size, y, *it2);

				// next line
				y += m_line_height;
			}
		} else
		{
			// next iterate on sections
			for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
				// reset
				it->m_rcSign.SetRectEmpty();
				it->m_rcTitle.SetRectEmpty();

				// is visible?
				it->m_rcTitle = CRect(0, y, w, y + m_line_height);
				CRect rcInter = it->m_rcTitle;
				rcInter.IntersectRect(rcInter, rc);
				if (m_display_mode == DM_CATEGORIZED && !rcInter.IsRectEmpty()) {
					// first shade rect
					if (m_shade_titles)
						dcMem.FillSolidRect(0, y, w, m_line_height, m_clrShade);

					// now draw a separator lines
					if (m_draw_lines) {
						dcMem.SelectPen(penShade);
						dcMem.MoveTo(0, y);
						dcMem.LineTo(w + 1, y);
						dcMem.MoveTo(0, y + m_line_height);
						dcMem.LineTo(w + 1, y + m_line_height);
					}

					// now draw gutter
					if (m_draw_gutter) {
						dcMem.SelectPen(penShade);
						dcMem.MoveTo(m_gutter_width, y);
						dcMem.LineTo(m_gutter_width, y + m_line_height + 1);
					}

					// now draw collapse sign
					int sign_top = y + margin + 2;
					dcMem.SelectPen(penTitle);
					it->m_rcSign = CRect(sign_left, sign_top, sign_left + sign_size + 1, sign_top + sign_size + 1);
					dcMem.FrameRect(it->m_rcSign, brushTitle);
					dcMem.MoveTo(sign_left + 2, sign_top + sign_size / 2);
					dcMem.LineTo(sign_left + 2 + sign_size / 2 + 1, sign_top + sign_size / 2);
					if (it->m_collapsed) {
						dcMem.MoveTo(sign_left + sign_size / 2, sign_top + 2);
						dcMem.LineTo(sign_left + sign_size / 2, sign_top + 2 + sign_size / 2 + 1);
					}

					// prepare draw text
					int title_left = sign_left + sign_size + 2 * margin;
					int title_top = y;
					dcMem.SelectFont(m_fntBold);
					it->m_rcTitle = CRect(title_left, title_top, w, title_top + m_line_height);

					// draw focus rect
					if (m_focused_section == it->m_id) {
						CSize sz;
						dcMem.GetTextExtent(it->m_title.c_str(), it->m_title.length(), &sz);
						int rect_left = title_left;
						int rect_top = title_top + (m_line_height - sz.cy) / 2;
						int rect_width = sz.cx + 3 * margin;
						int rect_height = sz.cy;
						dcMem.DrawFocusRect(CRect(rect_left, rect_top, rect_left + rect_width, rect_top + rect_height));
					}

					// now draw text
					dcMem.SetTextColor(m_clrTitle);
					dcMem.DrawText(it->m_title.c_str(), it->m_title.length(), CRect(title_left + GetTextMargin(), title_top, w, title_top + m_line_height), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
				}

				// next line
				if (m_display_mode == DM_CATEGORIZED)
					y += m_line_height;

				// iterate on items
				if (!it->m_collapsed || m_display_mode != DM_CATEGORIZED) {
					for (std::vector<CPropertyItem>::iterator it2 = it->m_items.begin(); it2 != it->m_items.end(); ++it2) {
						// reset
						it2->m_rcName.SetRectEmpty();
						it2->m_rcValue.SetRectEmpty();

						// is visible?
						it2->m_rcName = CRect(0, y, w, y + m_line_height);
						CRect rcInter = it2->m_rcName;
						rcInter.IntersectRect(rc, rcInter);
						if (!rcInter.IsRectEmpty())
							DrawItem(dcMem, w, sign_left + sign_size, y, it2);

						// next line
						y += m_line_height;
					}
				}
			}
		}
	}

	// Blt the changes to the screen DC.
	dc.BitBlt(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, dcMem, 0, 0, SRCCOPY);

	// Done with off-screen bitmap and DC.
	dcMem.SelectBitmap(oldBitmap);
	dcMem.SelectFont(oldFont);
	dcMem.SelectPen(oldPen);
	dcMem.SelectBrush(oldBrush);
	bmpMem.DeleteObject();
	dcMem.DeleteDC();

	// Validate All
	ValidateRgn(NULL);
	ValidateRect(NULL);

	return 0;
}

void CPropertyGrid::DrawItem(CDC& dc, int w, int x, int y, std::vector<CPropertyItem>::iterator& it) {
	// brush needed
	CBrush brushText;
	brushText.CreateSolidBrush(m_clrText);

	// pen needed
	CPen penShade;
	penShade.CreatePen(PS_SOLID, 1, m_clrShade);

	// to make sure we won't leak gdi resources
	auto oldBrush = dc.SelectBrush(brushText);
	auto oldPen = dc.SelectPen(penShade);
	auto oldFont = dc.SelectFont(m_fntNormal);

	// first shade rect
	if (m_shade_titles)
		dc.FillSolidRect(0, y, x + 2 * margin, m_line_height, m_clrShade);

	// now draw a separator line
	if (m_draw_lines) {
		dc.SelectPen(penShade);
		dc.MoveTo(0, y + m_line_height);
		dc.LineTo(w + 1, y + m_line_height);
	}

	// now draw gutter
	if (m_draw_gutter) {
		dc.SelectPen(penShade);
		dc.MoveTo(m_gutter_width, y);
		dc.LineTo(m_gutter_width, y + m_line_height + 1);
	}

	// needed
	int name_left = x + 2 * margin + GetTextMargin();
	int name_right = m_gutter_width - 1;
	int value_left = m_gutter_width;
	int value_right = w;

	// is being edited?
	if (m_focused_item == it->m_id && it->m_editable && GetEditMode(*it) != EM_CUSTOM) {
		value_right -= m_line_height;

		// the rect of the button
		m_rect_button = CRect(w - m_line_height, y, w, y + m_line_height);

		UINT pushed = m_button_depressed ? DFCS_PUSHED : 0;

		// now draw the button
		switch (GetEditMode(*it)) {
		case EM_MODAL:
			// draw a button
			dc.DrawFrameControl(m_rect_button, DFC_BUTTON, DFCS_BUTTONPUSH | pushed);
			dc.SelectFont(m_fntBold);
			dc.DrawText(L"...", 3, m_rect_button, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
			break;

		case EM_DROPDOWN:
			// draw an arrow
			dc.DrawFrameControl(m_rect_button, DFC_SCROLL, DFCS_SCROLLDOWN | pushed);
			break;

		case EM_INPLACE:
			// whole area is edit
			m_rect_button.left = m_gutter_width;
			break;

		default:
			ATLASSERT(FALSE);
		}
	}

	// update the rects
	it->m_rcName = CRect(0, y, m_gutter_width, y + m_line_height);
	it->m_rcValue = CRect(value_left, y, value_right, y + m_line_height);
	CRect rcValue = it->m_rcValue;
	rcValue.left += GetTextMargin();

	// focused
	if (m_focused_item == it->m_id) {
		int rect_left = name_left - 2 * margin;
		int rect_right = name_right;
		dc.FillSolidRect(rect_left, y, rect_right - rect_left + 1, m_line_height, m_clrFocus);
		dc.SetTextColor(m_clrHilite);
	} else
	{
		dc.SetTextColor(m_clrText);
	}

	// put name and value
	dc.SelectFont(m_fntNormal);
	dc.DrawText(it->m_name.c_str(), it->m_name.length(), CRect(name_left, y, name_right, y + m_line_height), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	// get back to normal text
	if (it->m_editable) dc.SetTextColor(m_clrEditable);
	else dc.SetTextColor(m_clrDisabled);

	// custom item
	if (it->m_type == IT_CUSTOM) {
		int save = dc.SaveDC();
		boost::get<ICustomItem*>(it->v)->DrawItem(dc, it->m_rcValue, m_focused_item == it->m_id);
		dc.RestoreDC(save);
	} else
	{
		// modified flag
		bool modified = false;

		// now draw text
		std::wstring strValue;

		switch (it->m_type) {
		case IT_TEXT:
		{
			strValue = wide(boost::get<std::string>(it->v));
			static const auto pattern = std::wregex(L"\r\n");
			strValue = std::regex_replace(strValue, pattern, L"�");
			break;
		}

		case IT_VARIANT:
		{
			strValue = wide(boost::get<CMyVariant>(it->v).ToString());
			break;
		}

		case IT_DATE:
		{
/*			std::string strTemp;
			if (it->m_options.size() && !it->m_options.front().empty()) strTemp = it->m_dtValue.Format(it->m_options.front().c_str());
			else strTemp = it->m_dtValue.Format(VAR_DATEVALUEONLY);
			strValue = LPCTSTR(strTemp);
			modified = (it->m_dtValue != it->m_dtValue_old); */
			break;
		}

		case IT_DATETIME:
		{
/*			std::string strTemp;
			if (it->m_options.size() && !it->m_options.front().empty()) strTemp = it->m_dtValue.Format(it->m_options.front().c_str());
			else strTemp = it->m_dtValue.Format();
			strValue = LPCTSTR(strTemp);
			modified = (it->m_dtValue != it->m_dtValue_old); */
			break;
		}

		case IT_BOOLEAN:
		{
			bool val = boost::get<CMyVariant>(it->v).as<bool>();
			bool old = boost::get<CMyVariant>(it->v_old).as<bool>();
			strValue = val ? m_strTrue : m_strFalse;
			modified = (val != old);
			break;
		}

		case IT_COMBO:
		{
	//		if (it->m_nValue >= 0 && it->m_nValue<int(it->m_options.size()))
	//			strValue = it->m_options[it->m_nValue];
	//		modified = (it->m_nValue != it->m_nValue_old);
	//		break;
		}

		case IT_FILE:
		case IT_FOLDER:
		{
		//	TCHAR szBuffer[1024];
			
		//	strncpy_s(szBuffer, strValue, 1024);
		//	PathCompactPath(dc.m_hDC, szBuffer, rcValue.Width());
		//	strValue = szBuffer;
			break;
		}

		case IT_COLOR:
		{
			// draw a sample rectangle
	/*		CRect rc = rcValue;
			rc.DeflateRect(0, 2, 0, 2);
			rc.top++;
			rc.right = rc.left + m_line_height;
			dc.FrameRect(rc, brushText);
			rc.DeflateRect(1, 1);
			dc.FillSolidRect(rc, it->m_clrValue);
			rcValue.left = rc.right + 3 * margin;

			// update the text
			std::string strTemp;
			strTemp.Format(_T("%d; %d; %d"), GetRValue(it->m_clrValue), GetGValue(it->m_clrValue), GetBValue(it->m_clrValue));
			strValue = LPCTSTR(strTemp);
			modified = (it->m_clrValue != it->m_clrValue_old);
			*/
			break;
		}

		case IT_FONT:
		{
	/*		std::string strTemp;
			strTemp.Format(_T("%s; %dpt"), it->m_lfValue.lfFaceName, -MulDiv(it->m_lfValue.lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY)));
			strValue = LPCTSTR(strTemp);
			modified = (memcmp(&it->m_lfValue, &it->m_lfValue_old, sizeof(LOGFONT)) != 0);
			*/
			break;
		}
		}

		// set proper font
		if (modified && m_bold_modified) 
			dc.SelectFont(m_fntBold);
		else if (it->m_editable && m_bold_editables) 
			dc.SelectFont(m_fntBold);
		else 
			dc.SelectFont(m_fntNormal);

		// now draw it

		dc.DrawText(strValue.c_str(), strValue.length(), rcValue, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}

	// clean up
	dc.SelectFont(oldFont);
	dc.SelectPen(oldPen);
	dc.SelectBrush(oldBrush);
}

//
// mouse interaction
//

LRESULT CPropertyGrid::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// destroy edit
	SetFocus();
	DeleteEditControl();
	
	CPoint point(lParam);
	
	// click on button?
	if (m_rect_button.PtInRect(point)) {
		m_button_pushed = true;
		m_button_depressed = true;
		SetCapture();
		Invalidate();
		return 0;
	}

	// click on button?
	if (m_focused_item != -1) {
		auto item = FindItem(m_focused_item);
		if (item && item->m_type == IT_CUSTOM
			&& GetEditMode(*item) == EM_CUSTOM
			&& boost::get<ICustomItem*>(item->v)->OnLButtonDown(item->m_rcValue, point)) {
			m_custom_tracking = true;
			SetCapture();
			Invalidate();
			return 0;
		}
	}

	// resizing gutter?
	if (abs(point.x - m_gutter_width) < 3) {
		::SetCursor(global.GetCursor(AlphaCursor::SizeWE));
		m_resizing_gutter = true;
		m_user_has_resized_gutter = true;
		m_ptLast = point;
		SetCapture();
		Invalidate();
		return 0;
	}

	// disable focus
	m_focused_item = -1;
	m_focused_section = -1;
	m_rect_button.SetRectEmpty();

	// did we click on a section
	if (m_display_mode == DM_CATEGORIZED) {
		for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
			if (it->m_rcSign.PtInRect(point)) {
				it->m_collapsed = !it->m_collapsed;
				m_focused_section = it->m_id;
				RecalcLayout();
				return 0;
			} else if (it->m_rcTitle.PtInRect(point)) {
				m_focused_section = it->m_id;
				Invalidate();
				return 0;
			}
		}
	}

	// focus
	for (const auto& sec : m_sections) {
		if (!sec.m_collapsed || m_display_mode != DM_CATEGORIZED) {
			for (const auto& item : sec.m_items) {
				BOOL hitName = item.m_rcName.PtInRect(point);
				BOOL hitValue = item.m_rcValue.PtInRect(point);
				if (hitName || hitValue) {
					if (item.m_editable && m_focus_disabled) {
						m_focused_item = item.m_id;
						if (hitValue) {
							m_value_clicked = (GetEditMode(item) == EM_INPLACE || GetEditMode(item) == EM_DROPDOWN);
						}
					}
					Invalidate();
					return 0;
				}
			}
		}
	}

	Invalidate();
	return 0;
}

LRESULT CPropertyGrid::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (m_focused_item != -1) {
		auto item = FindItem(m_focused_item);

		if (item) {
			if (item->m_type == IT_BOOLEAN) {
				auto& vt = boost::get<CMyVariant>(item->v);
				vt = !vt.as<bool>();
				m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
				Invalidate();
			} else if (item->m_type == IT_COMBO) {
			//	item->m_nValue = (item->m_nValue + 1) % int(item->m_options.size());
			//	m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->data);
				Invalidate();
			} else if (GetEditMode(*item) == EM_MODAL) {
				EditFocusedItem();
			}
		}
	} else if (m_focused_section != -1) {
		CSection* pSection = FindSection(m_focused_section);
		if (pSection) {
			pSection->m_collapsed = !pSection->m_collapsed;
			Invalidate();
		}
	}

	return 0;
}

LRESULT CPropertyGrid::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	if (m_custom_tracking) {
		auto item = FindItem(m_focused_item);
		if (item) {
			boost::get<ICustomItem*>(item->v)->OnMouseMove(item->m_rcValue, point);
			Invalidate();
		}
	} else if (m_button_pushed) {
		m_button_depressed = m_rect_button.PtInRect(point) ? true : false;
		Invalidate();
	} else if (m_resizing_gutter) {
		::SetCursor(global.GetCursor(AlphaCursor::SizeWE));
		m_gutter_width += point.x - m_ptLast.x;
		CRect rc;
		GetClientRect(&rc);
		if (m_gutter_width < rc.Width() / 5) 
			m_gutter_width = rc.Width() / 5;
		if (m_gutter_width > 4 * rc.Width() / 5) 
			m_gutter_width = 4 * rc.Width() / 5;
		m_ptLast = point;
		Invalidate();
	} else if (!m_control) {
		if (abs(point.x - m_gutter_width) < 3) 
			::SetCursor(global.GetCursor(AlphaCursor::SizeWE));
		else 
			::SetCursor(global.GetCursor(AlphaCursor::Arrow));
	}

	return 0;
}

LRESULT CPropertyGrid::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	if (m_custom_tracking) {
		m_custom_tracking = false;
		ReleaseCapture();
		Invalidate();
		auto item = FindItem(m_focused_item);
		if (item) {
			boost::get<ICustomItem*>(item->v)->OnLButtonUp(item->m_rcValue, point);
		}
	} else if (m_button_pushed || m_value_clicked) {
		m_button_pushed = false;
		m_button_depressed = false;
		ReleaseCapture();
		Invalidate();

		if (m_rect_button.PtInRect(point) || (m_value_clicked && m_focused_item != -1 && FindItem(m_focused_item) && FindItem(m_focused_item)->m_rcValue.PtInRect(point))) {
			m_value_clicked = false;
			auto item = FindItem(m_focused_item);
			if (item) {
				if (GetEditMode(*item) == EM_DROPDOWN) {
					if (item->m_type == IT_CUSTOM) {
						CRect rc = m_rect_button;
						rc.left = m_gutter_width;
						boost::get<ICustomItem*>(item->v)->ShowDropDown(rc);
					} else if (item->m_type == IT_DATE) {
/*						// the calendar rect
						CRect rc = m_rect_button;
						rc.left = m_gutter_width;
						rc.top += m_line_height;
						rc.bottom = rc.top + 100;
						ClientToScreen(&rc);

						// create it
						m_control = new CPropertyGridMonthCalCtrl;
						CPropertyGridMonthCalCtrl* mc = (CPropertyGridMonthCalCtrl*)m_control;
						mc->CreateEx(0, MONTHCAL_CLASS, NULL, WS_POPUP | WS_BORDER, rc, GetParent(), 0);
						mc->SetCurSel(pItem->m_dtValue);
						mc->SetOwner(this);
						mc->SizeMinReq();

						// now position it
						CRect rc2;
						mc->GetWindowRect(&rc2);
						rc2.OffsetRect(rc.right - rc2.right, 0);
						mc->SetWindowPos(NULL, rc2.left, rc2.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
						*/
					} else
					{
						// the combo rect
						CRect rc = m_rect_button;
						rc.left = m_gutter_width;
						rc.top += m_line_height;
						rc.bottom = rc.top + 100;

						// create it
						m_control = &m_combo;
						
						m_combo.Create(m_hWnd, rc, WS_POPUP, 0);
						m_combo.SetColors(m_clrBack, m_clrText, m_clrFocus, m_clrHilite);
						m_combo.SetFont(m_fntNormal.m_hFont);

						if (item->m_type == IT_BOOLEAN) {						
							m_combo.AddString(m_strFalse);
							m_combo.AddString(m_strTrue);
							m_combo.SetCurSel(boost::get<CMyVariant>(item->v).as<bool>());
						} else
						{
						//	for (std::vector<std::string>::iterator it = item->m_options.begin(); it != pItem->m_options.end(); ++it)
						//		m_combo.AddString(*it);
						//	if (!item->m_undefined)
						//		m_combo.SetCurSel(pItem->m_nValue);
						}
						m_combo.ShowWindow(SW_SHOW);
					}
				} else if (GetEditMode(*item) == EM_INPLACE) {
					// the in-place edit rect
					CRect rc = m_rect_button;
					rc.left++;
					rc.top += margin;

					// the value
					std::string strValue;
					if (item->m_type == IT_VARIANT) {
						strValue = boost::get<CMyVariant>(item->v).ToString();
					
					} else if (item->m_type == IT_CUSTOM) {
				//		strValue = item->m_pCustom->GetStringForInPlaceEdit();
					} else
					{
						ATLASSERT(FALSE);
					}

					// create it
					
					m_edit.Create(m_hWnd, rc, strValue, WS_CHILD | ES_AUTOHSCROLL, 1000);
					m_edit.SetColors(m_clrBack, m_clrText);
					m_edit.SetFont(m_fntNormal);
					m_edit.ShowWindow(SW_SHOW);

					m_control = &m_edit;
				} else if (GetEditMode(*item) == EM_MODAL) {
					EditFocusedItem();
				} else if (GetEditMode(*item) == EM_CUSTOM) {
			//		item->m_pCustom->OnLButtonUp(item->m_rcValue, point);
				} else
				{
					ATLASSERT(FALSE);
				}
			}
		}
	} else if (m_resizing_gutter) {
		ReleaseCapture();
		m_resizing_gutter = false;
		::SetCursor(global.GetCursor(AlphaCursor::Arrow));
	}

	return 0;
}

//
// keyboard interaction
//


LRESULT CPropertyGrid::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return DLGC_WANTCHARS | DLGC_WANTARROWS;
}


void CPropertyGrid::MoveForward(HSECTION& focused_section, HITEM& focused_item) {
	for (int pass = 0; pass < 2; pass++) {
		bool found = false;

		bool stop_on_next_valid = false;
		if (focused_section == -1 && focused_item == -1)
			stop_on_next_valid = true;

		for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
			if (m_display_mode == DM_CATEGORIZED) {
				if (it->m_id == focused_section) {
					stop_on_next_valid = true;
				} else if (stop_on_next_valid) {
					focused_section = it->m_id;
					focused_item = -1;
					found = true;
					break;
				}
			}

			if (!it->m_collapsed || m_display_mode != DM_CATEGORIZED) {
				for (std::vector<CPropertyItem>::iterator it2 = it->m_items.begin(); it2 != it->m_items.end(); ++it2) {
					if (it2->m_id == focused_item) {
						stop_on_next_valid = true;
					} else if (stop_on_next_valid) {
						if (it2->m_editable || m_focus_disabled) {
							focused_section = -1;
							focused_item = it2->m_id;
							found = true;
							break;
						}
					}
				}

				if (found)
					break;
			}
		}

		if (found)
			break;

		focused_section = -1;
		focused_item = -1;
	}
}

void CPropertyGrid::FocusNextItem() {
	// simple move forward
	MoveForward(m_focused_section, m_focused_item);

	// ensure visible
	CRect rc;
	if (m_focused_section != -1 && FindSection(m_focused_section)) 
		rc = FindSection(m_focused_section)->m_rcTitle;
	else if (m_focused_item != -1 && FindItem(m_focused_item)) 
		rc = FindItem(m_focused_item)->m_rcName;
	if (!rc.IsRectEmpty()) {
		CRect rect;
		GetClientRect(&rect);
		rect.IntersectRect(rc, rect);
		BOOL bHandled = TRUE;
		if (rect.Height() != m_line_height)
			OnVScroll(WM_VSCROLL, MAKEWPARAM(rc.top, SB_THUMBPOSITION), (LPARAM)m_scrollbar.m_hWnd, bHandled);
	}

	// done
	Invalidate();
}

void CPropertyGrid::FocusPrevItem() {
	for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
		if (m_display_mode == DM_CATEGORIZED) {
			HSECTION focused_section = it->m_id;
			HITEM focused_item = -1;
			MoveForward(focused_section, focused_item);
			if (focused_section == m_focused_section && focused_item == m_focused_item) {
				m_focused_section = it->m_id;
				m_focused_item = -1;
				break;
			}
		}

		if (!it->m_collapsed || m_display_mode != DM_CATEGORIZED) {
			bool found = false;
			for (std::vector<CPropertyItem>::iterator it2 = it->m_items.begin(); it2 != it->m_items.end(); ++it2) {
				if (!it2->m_editable && !m_focus_disabled)
					continue;

				HSECTION focused_section = -1;
				HITEM focused_item = it2->m_id;
				MoveForward(focused_section, focused_item);
				if (focused_section == m_focused_section && focused_item == m_focused_item) {
					m_focused_section = -1;
					m_focused_item = it2->m_id;
					found = true;
					break;
				}
			}

			if (found)
				break;
		}
	}

	// ensure visible
	CRect rc(0, 0, 0, 0);
	if (m_focused_section != -1 && FindSection(m_focused_section)) rc = FindSection(m_focused_section)->m_rcTitle;
	else if (m_focused_item != -1 && FindItem(m_focused_item)) rc = FindItem(m_focused_item)->m_rcName;
	if (!rc.IsRectEmpty()) {
		CRect rect;
		GetClientRect(&rect);
		rect.IntersectRect(rc, rect);
		BOOL bHandled = TRUE;
		if (rect.Height() != m_line_height)
			OnVScroll(WM_VSCROLL, MAKEWPARAM(rc.top, SB_THUMBPOSITION), (LPARAM)m_scrollbar.m_hWnd, bHandled);
	}

	// done
	Invalidate();
}

LRESULT CPropertyGrid::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	TCHAR nChar = (TCHAR)wParam;
	if (nChar == _T('*')) {
		ExpandAll(true);
	} else if (nChar == _T('/')) {
		ExpandAll(false);
	} else if (nChar == _T('+') || nChar == _T('-')) {
		if (m_focused_section != -1) {
			CSection* pSection = FindSection(m_focused_section);
			if (pSection) pSection->m_collapsed = (nChar == _T('-'));
			RecalcLayout();
		}
	}

	bHandled = FALSE;

	return 0;
}

LRESULT CPropertyGrid::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (wParam == VK_DOWN) {
		FocusNextItem();
	} else if (wParam == VK_UP) {
		FocusPrevItem();
	} else if (wParam == VK_LEFT) {
		if (m_focused_section != -1 && FindSection(m_focused_section) && FindSection(m_focused_section)->m_collapsed == false) {
			ExpandSection(m_focused_section, false);
		} else
		{
			FocusPrevItem();
		}
	} else if (wParam == VK_RIGHT) {
		if (m_focused_section != -1 && FindSection(m_focused_section) && FindSection(m_focused_section)->m_collapsed == true) {
			ExpandSection(m_focused_section, true);
		} else
		{
			FocusNextItem();
		}
	}

	bHandled = FALSE;

	return 0;
}

//
// scrolling
//

void CPropertyGrid::RecalcLayout() {
	// save current scroll offset
	int offset = GetScrollOffset();

	// total height
	int height = 0;
	for (std::vector<CSection>::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
		if (m_display_mode == DM_CATEGORIZED)
			height += m_line_height;
		if (!it->m_collapsed || m_display_mode != DM_CATEGORIZED)
			height += int(it->m_items.size())*m_line_height;
	}

	// client rect
	CRect rc;
	GetClientRect(&rc);
	if (height < rc.Height()) {
		if (m_scrollbar.m_hWnd != NULL) {
			m_scrollbar.EnableScrollBar(ESB_DISABLE_BOTH);
			m_scrollbar.ShowScrollBar(FALSE);
		}
		m_scroll_enabled = false;
	} else
	{
		if (m_scrollbar.m_hWnd == NULL) {
			CRect rect = rc;
			rect.left = rect.right - GetSystemMetrics(SM_CXVSCROLL);
			m_scrollbar.Create(m_hWnd, rect, NULL, WS_CHILD | SBS_VERT, 0, 1000);
		}

		m_scrollbar.EnableScrollBar(ESB_ENABLE_BOTH);

		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = height;
		info.nPage = rc.Height();
		info.nPos = std::min(offset, height);
		info.nTrackPos = 2;
		m_scrollbar.SetScrollInfo(&info);

		m_scrollbar.ShowScrollBar();
		m_scroll_enabled = true;
	}

	if (m_hWnd)
		Invalidate();
}

int CPropertyGrid::GetScrollOffset() {
	if (m_scrollbar && m_scrollbar.IsWindowEnabled() == TRUE)
		return m_scrollbar.GetScrollPos();
	return 0;
}

LRESULT CPropertyGrid::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int nPos = LOWORD(wParam);
	int nSBCode = HIWORD(wParam);
	CScrollBar scrollbar = (HWND)lParam;
	// check
	if (!m_scroll_enabled) 
		return 0;
	
	if (scrollbar != m_scrollbar)
		return 0;
	
	if (nSBCode == SB_ENDSCROLL)
		return 0;

	
	// set focus to us
	SetFocus();

	// get the scroll info
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_ALL;
	m_scrollbar.GetScrollInfo(&info);
	int min = info.nMin;
	int pos = info.nPos;
	int max = info.nMax - info.nPage;

	// the entire rect
	CRect rect;
	GetClientRect(&rect);
	int h = rect.Height();

	// the rect without the scrollbar
	CRect rc(0, 0, rect.right - GetSystemMetrics(SM_CXVSCROLL), rect.bottom);

	switch (nSBCode) {
	case SB_TOP:
		scrollbar.SetScrollPos(min);
		break;

	case SB_BOTTOM:
		scrollbar.SetScrollPos(max);
		break;

	case SB_LINEDOWN:
		if (pos + m_line_height >= max) 
			scrollbar.SetScrollPos(max);
		else 
			scrollbar.SetScrollPos(pos + m_line_height);
		break;

	case SB_LINEUP:
		if (pos - m_line_height <= min) 
			scrollbar.SetScrollPos(min);
		else 
			scrollbar.SetScrollPos(pos - m_line_height);
		break;

	case SB_PAGEDOWN:
		if (pos + h >= max) 
			scrollbar.SetScrollPos(max);
		else 
			scrollbar.SetScrollPos(pos + h);
		break;

	case SB_PAGEUP:
		if (pos - h <= min) 
			scrollbar.SetScrollPos(min);
		else 
			scrollbar.SetScrollPos(pos - h);
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		int diff = nPos - pos;
		if (diff == 0)
			return 0;
		if (pos <= min && diff < 0) 
			return 0;
		if (pos >= max && diff > 0) 
			return 0;
		scrollbar.SetScrollPos(nPos);
	}

	Invalidate();

	return 0;
}

LRESULT CPropertyGrid::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

	int steps = abs(zDelta) / WHEEL_DELTA;

	for (int i = 0; i < 3 * steps; i++) {
		BOOL bHandled = TRUE;
		if (zDelta > 0) 
			OnVScroll(WM_VSCROLL, MAKEWPARAM(0, SB_LINEUP), (LPARAM)m_scrollbar.m_hWnd, bHandled);
		if (zDelta < 0) 
			OnVScroll(WM_VSCROLL, MAKEWPARAM(0, SB_LINEDOWN), (LPARAM)m_scrollbar.m_hWnd, bHandled);
	}

	return 0;
}

//
// editing
//

CPropertyGrid::EEditMode CPropertyGrid::GetEditMode(const CPropertyItem& item) {
	switch (item.m_type) {
	case IT_CUSTOM:
		return boost::get<ICustomItem*>(item.v)->GetEditMode();

	case IT_VARIANT:
		return EM_INPLACE;

	case IT_COMBO:
	case IT_BOOLEAN:
	case IT_DATE:
		return EM_DROPDOWN;

	case IT_TEXT:
	case IT_DATETIME:
	case IT_FILE:
	case IT_FOLDER:
	case IT_COLOR:
	case IT_FONT:
		return EM_MODAL;

	default:
		ATLASSERT(FALSE);
		return EM_CUSTOM;
	}
}

void CPropertyGrid::DeleteEditControl() {
	// destroy edit
	if (m_control != nullptr) {
		if (m_control->m_hWnd) {
			m_control->DestroyWindow();
		}
		m_control = nullptr;
	}
}

LRESULT CPropertyGrid::OnComboSelChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	auto item = FindItem(m_focused_item);
	if (item) {
		
		if (item->m_type == IT_BOOLEAN) {
			boost::get<CMyVariant>(item->v) = (bool)wParam;
			m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
			DeleteEditControl();
			Invalidate();
		} else if (item->m_type == IT_COMBO) {
			boost::get<ComboData>(item->v).index = (int)wParam;
			m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
			DeleteEditControl();
			Invalidate();
		} else
		{
			ATLASSERT(FALSE);
		}
		
	}
	return 0;
}

LRESULT CPropertyGrid::OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	auto item = FindItem(m_focused_item);
	if (item) {
		if (item->m_type == IT_VARIANT) {
			try {
				boost::get<CMyVariant>(item->v).Parse(*(std::string*)wParam);
				m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
				DeleteEditControl();
				Invalidate();
			} catch (input_error& e) {
				e.msg_box(this->m_hWnd);
			}
		} else if (item->m_type == IT_TEXT) {
			boost::get<std::string>(item->v) = *(std::string*)wParam;
			m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
			DeleteEditControl();
			Invalidate();
		} else if (item->m_type == IT_CUSTOM) {
			if (boost::get<ICustomItem*>(item->v)->OnItemEdited(*(std::string*)wParam))
				m_owner.SendMessage(WM_PG_ITEMCHANGED, (WPARAM)&item->v, item->tag);
			DeleteEditControl();
			Invalidate();
		} else
		{
			ATLASSERT(FALSE);
		}
	}
	return 0;
}

LRESULT CPropertyGrid::OnDateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	/*
	CPropertyItem* pItem = FindItem(m_focused_item);
	if (pItem) {
		if (pItem->m_type == IT_DATE) {
			CPropertyGridMonthCalCtrl* mc = (CPropertyGridMonthCalCtrl*)m_control;
			mc->GetCurSel(pItem->m_dtValue);
			pItem->m_undefined = false;
			GetParent().SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
			DeleteEditControl();
			Invalidate();

		} else
		{
			ATLASSERT(FALSE);
		}
	}
	*/
	return 0;
}

void CPropertyGrid::EditFocusedItem() {
	CPropertyItem* pItem = FindItem(m_focused_item);
	if (pItem) {
		if (!pItem->m_editable)
			return;

		if (pItem->m_type == IT_TEXT) {
/*			CDynDialogEx dlg(GetParent());
			dlg.SetUseSystemButtons(FALSE);
			dlg.SetWindowTitle(pItem->m_name);
			dlg.SetFont(&m_fntNormal);
			std::string strValue = pItem->m_strValue.c_str();
			dlg.AddDlgControl("EDIT", pItem->m_strValue.c_str(), STYLE_EDIT | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_WANTRETURN, EXSTYLE_EDIT, CRect(7, 7, 200, 100), (void*)&strValue);
			dlg.AddDlgControl("BUTTON", m_strOk.c_str(), STYLE_BUTTON, EXSTYLE_BUTTON, CRect(56, 106, 106, 120), NULL, IDOK);
			dlg.AddDlgControl("BUTTON", m_strCancel.c_str(), STYLE_BUTTON, EXSTYLE_BUTTON, CRect(110, 106, 160, 120), NULL, IDCANCEL);
			if (dlg.DoModal() == IDOK) {
				pItem->m_strValue = LPCTSTR(strValue);
				pItem->m_undefined = false;
				GetParent().SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else if (pItem->m_type == IT_DATETIME) {
/*			CDynDialogEx dlg(GetParent());
			dlg.SetUseSystemButtons(FALSE);
			dlg.SetWindowTitle(pItem->m_name.c_str());
			dlg.SetFont(&m_fntNormal);
			COleDateTime dtValueDate = pItem->m_dtValue;
			CTime dtValueTime(pItem->m_dtValue.GetYear(), pItem->m_dtValue.GetMonth(), pItem->m_dtValue.GetDay(), pItem->m_dtValue.GetHour(), pItem->m_dtValue.GetMinute(), pItem->m_dtValue.GetSecond());
			dlg.AddDlgControl("STATIC", m_strDate.c_str(), STYLE_STATIC, EXSTYLE_STATIC, CRect(7, 3, 60, 12));
			dlg.AddDlgControl("STATIC", m_strTime.c_str(), STYLE_STATIC, EXSTYLE_STATIC, CRect(67, 3, 120, 12));
			dlg.AddDlgControl("SysDateTimePick32", "", STYLE_DATETIMEPICKER | DTS_SHORTDATEFORMAT, EXSTYLE_DATETIMEPICKER, CRect(7, 13, 60, 26), (void*)&dtValueDate);
			dlg.AddDlgControl("SysDateTimePick32", "", STYLE_DATETIMEPICKER | DTS_TIMEFORMAT, EXSTYLE_DATETIMEPICKER, CRect(67, 13, 120, 26), (void*)&dtValueTime);
			dlg.AddDlgControl("BUTTON", m_strOk.c_str(), STYLE_BUTTON, EXSTYLE_BUTTON, CRect(7, 37, 60, 51), NULL, IDOK);
			dlg.AddDlgControl("BUTTON", m_strCancel.c_str(), STYLE_BUTTON, EXSTYLE_BUTTON, CRect(67, 37, 120, 51), NULL, IDCANCEL);
			if (dlg.DoModal() == IDOK) {
				pItem->m_dtValue.SetDateTime(dtValueDate.GetYear(), dtValueDate.GetMonth(), dtValueDate.GetDay(),
					dtValueTime.GetHour(), dtValueTime.GetMinute(), dtValueTime.GetSecond());
				pItem->m_undefined = false;
				GetOwner()->SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else if (pItem->m_type == IT_COLOR) {
	/*		CColorDialog dlg(pItem->m_clrValue, 0, GetParent());
			if (dlg.DoModal() == IDOK) {
				pItem->m_clrValue = dlg.GetColor();
				pItem->m_undefined = false;
				m_owner.SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}*/
		} else if (pItem->m_type == IT_FILE) {
	/*		CFileDialog dlg(TRUE, NULL, pItem->m_strValue, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, pItem->m_options.front(), GetParent());
			if (dlg.DoModal() == IDOK) {
				std::string& str = pItem->m_strValue;
				str.GetBuffer(MAX_PATH);
				int len = dlg.GetFilePath(str.GetBuffer(MAX_PATH), MAX_PATH);
				str.ReleaseBuffer(len - 1);
				pItem->m_undefined = false;
				m_owner.SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else if (pItem->m_type == IT_FOLDER) {
/*			CPropertyGridDirectoryPicker::m_strTitle = pItem->m_options.front();
			if (CPropertyGridDirectoryPicker::PickDirectory(pItem->m_strValue, GetParent()->m_hWnd)) {
				pItem->m_undefined = false;
				GetParent().SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else if (pItem->m_type == IT_FONT) {
	/*		CFontDialog dlg(&pItem->m_lfValue, CF_EFFECTS | CF_SCREENFONTS, NULL, GetParent());
			if (dlg.DoModal() == IDOK) {
				memcpy(&pItem->m_lfValue, dlg.m_cf.lpLogFont, sizeof(LOGFONT));
				pItem->m_undefined = false;
				m_owner.SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else if (pItem->m_type == IT_CUSTOM) {
		/*	if (pItem->m_pCustom->OnEditItem()) {
				m_owner.SendMessage(WM_PG_ITEMCHANGED, pItem->m_id);
				Invalidate();
			}
			*/
		} else
		{
			ATLASSERT(FALSE);
		}
	}
}
