// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "global.h"
#include "onlinetreeitem.h"
#include <npsys/fat_data_callback.h>

class CTreeItemAbstract;

class CMyOnlineTreeView 
	: public CWindowImpl<CMyOnlineTreeView, CWindow>
	, public virtual IFatDataCallBack {
	using base = CWindowImpl<CMyOnlineTreeView, CWindow>;

	static constexpr auto header_height = 30;
public:
	DECLARE_WND_CLASS(NULL);
	
	BEGIN_MSG_MAP(CMyOnlineTreeView)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	struct Item {
		std::wstring name;
		int pos;
		CRect rc;
		void SetPos(int percent, int width) {
			pos = int((float)percent / 100.f * (float)width);
			rc.left = pos - 5;
			rc.right = pos + 5;
		}
		void SetPosDx(int dx) {
			pos += dx;
			rc.left = pos - 5;
			rc.right = pos + 5;
		}
	};

	using item_list = std::array<Item, 2>;

	CMyOnlineTreeView();

	HWND Create(CWindow parent, CRect& rect);
	void CreateItems(CTreeItemAbstract* root);

	class CTreeWrapper : public CWindowImpl<CTreeWrapper, CWindow> {
		using base = CWindowImpl<CTreeWrapper, CWindow>;
	public:
		DECLARE_WND_CLASS(NULL);
		BEGIN_MSG_MAP(CTreeWrapper)
		END_MSG_MAP()
	};

	class CActualTreeView : public CWindowWithReflectorImpl<CActualTreeView, CTreeViewCtrl>
	{
		using base = CWindowWithReflectorImpl<CActualTreeView, CTreeViewCtrl>;
	public:
		DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())

		BEGIN_MSG_MAP(CActualTreeView)
			REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTvnItemExpanding)
			REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnNMCustomDraw)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			CHAIN_MSG_MAP(base)
		END_MSG_MAP()

		CActualTreeView(CMyOnlineTreeView& main, const item_list& items);


		HWND Create(HWND hParent, CRect& rc);
	protected:
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnTvnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		const item_list& items_;
		COLORREF line_color_selected_;
		CMyOnlineTreeView& main_;
	};

	class CHeaderView : public CWindowImpl<CHeaderView, CWindow> {
	public:
		DECLARE_WND_CLASS(NULL);

		BEGIN_MSG_MAP(CHeaderView)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
		END_MSG_MAP()

		CHeaderView(CActualTreeView& tree, item_list& items)
			: tree_(tree)
			, items_(items)
		{
		}

	private:
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		NPSystemCursor cursor_ = NPSystemCursor::Arrow;

		size_t GetSelected(CPoint& pt) const noexcept;

		CPen pen_;
		int prev_pos_;
		int height_;
		int diff_;
		size_t drag_ix_ = -1;

		CActualTreeView& tree_;
		item_list& items_;
		CRect rc_header_;
	};
	CActualTreeView tree;
private:
	friend CActualTreeView;

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	virtual size_t AdviseImpl();
	virtual void OnDataChangedImpl(nprpc::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a);
	virtual void OnConnectionLost() noexcept;

	CTreeWrapper wrapper_;
	CHeaderView header_;
	item_list items_;
	
	std::vector<std::unique_ptr<COnlineTreeItem>> nodes_;
	std::vector<COnlineTreeItem*> online_values_;
	std::unordered_map<nps::var_handle, COnlineTreeItem*> m_ref;
};