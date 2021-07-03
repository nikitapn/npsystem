#pragma once

#include "Stuff.h"
#include "MyTabView.h"
#include "Global.h"
#include "constants.h"
#include "dockable.h"


class CTreeItemAbstract;
class CTreeSystem;

class CMyTreeView : public CDockableWindowWithReflectorImpl<CMyTreeView, CTreeViewCtrlT<CDockableWindowT<CMyTreeView, DockIndex::SystemTree>>>
{
	using base = CDockableWindowWithReflectorImpl<CMyTreeView, CTreeViewCtrlT<CDockableWindowT<CMyTreeView, DockIndex::SystemTree>>>;
public:
	DECLARE_WND_SUPERCLASS(L"CMyTreeView", CTreeViewCtrlT<CDockableWindow>::GetWndClassName())

	CMyTreeView(CMyTabView& tabView) 
		: m_tabview(tabView)
	{
		m_state.dock = Dock::Left;
	}

	HWND Create(
		_In_opt_ HWND hWndParent,
		_In_ _U_RECT rect = NULL,
		_In_opt_z_ LPCTSTR szWindowName = NULL,
		_In_ DWORD dwStyle = 0,
		_In_ DWORD dwExStyle = 0,
		_In_ _U_MENUorID MenuOrID = 0U,
		_In_opt_ LPVOID lpCreateParam = NULL);

	BEGIN_MSG_MAP(CMyTreeView)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTvnItemExpanding)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINLABELEDIT, OnTvnBeginLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ENDLABELEDIT, OnTvnEndLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, OnTvnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnNMCustomDraw)
		//
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDeleteItem)
		COMMAND_ID_HANDLER(ID_ITEM_DELETE, OnDeleteItem)
		COMMAND_RANGE_HANDLER(0xB000, 0xD000, OnCommand)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	void LoadTree();
	void UnSelect();
	LRESULT OnTvnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTvnBeginLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTvnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTvnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDeleteItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
public:
	void SaveTree();
	void InvalidateItem(CTreeItemAbstract* pItem);
	void UpdateMetrics();
private:
	CTreeSystem* m_root = nullptr;
	CMyTabView& m_tabview;
	bool m_loaded = false;
	HACCEL m_hAccel;
	BOOL m_bDrag;
	BOOL m_bDragResult;
	CTreeItemAbstract* m_pDestItem = nullptr;

	BOOL m_bFocus;
	HTREEITEM m_hLast;
	BOOL m_labelEdit;
	HTREEITEM m_hDragItemOld;
	HTREEITEM m_lastSelected;

	std::unordered_set<HTREEITEM> m_selectedItems;

	HBRUSH m_hBackBrush;
	CBrush m_brButtonExpanded;
	CBrush m_brButtonCollapsed;
	CBrush m_brButtonHovered;
	AlphaCursor m_MouseCursor;

	CImageList m_imageList;
	CFont m_itemFont;
	CImageList* m_pDragImage;
	CImageList m_DragImage;
	CPoint ptPrevCursor;
	int m_fMultiSelect = 0;

	static constexpr auto button_cx = 10;
	static constexpr auto button_cy = 10;
	static constexpr auto button_y_offset = (constants::treeview::label_cy - button_cy) / 2;

	HTREEITEM m_buttonHovered = NULL;
	CBrush m_brButton;
	CBrush m_brButtonFocused;
	CRgn m_rgnButtonExp;
	CRgn m_rgnButtonCol;

	void CreateLogicButtonRectFromTextRect(CRect& rc);
};