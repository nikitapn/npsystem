// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "View.h"
#include "mytabview.h"

class InteractiveItem;

struct MYLVITEM {
	std::wstring_view colName;
	std::string_view format;
	int len;

	MYLVITEM(std::wstring_view _column_name, std::string_view _format, int _len)
		: colName(_column_name)
		, format(_format)
		, len(_len)
	{
	}
};

struct MYLVITEMDATA {
	LPARAM lParam;
	std::array<LPARAM, 10> values;

	template<class... T>
	MYLVITEMDATA(LPARAM _lParam, T... vals) 
		: lParam{ _lParam }
		, values{ vals... }
	{
	}
};

class CViewAdapter {
public:
	CViewAdapter(CTreeItemAbstract* item)
		: item_(item) {}
	virtual ~CViewAdapter() = default;
	CTreeItemAbstract* GetItem() { return item_; }

	std::vector<MYLVITEM> m_columns;
	std::vector<MYLVITEMDATA> m_data;
private:
	CTreeItemAbstract* item_;
};

template<typename T>
class CViewAdapterTempData : public CViewAdapter {
public:
	CViewAdapterTempData(CTreeItemAbstract* item) 
		: CViewAdapter(item) {}
	
	std::vector<std::unique_ptr<T>> temp_data;
};

class CMyListViewCtrl : public CListViewCtrl
{
public:
	CMyListViewCtrl(HWND hWnd = NULL) : CListViewCtrl(hWnd) { }

	CMyListViewCtrl& operator =(HWND hWnd) {
		this->m_hWnd = hWnd;
		return *this;
	}
	void AddColumn(std::wstring_view col_name, int len) noexcept {
		ASSERT(col_name.length() < 50);
		LVCOLUMN lvColumn;

		lvColumn.cchTextMax = 50;
		lvColumn.cx = 0;
		lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
		lvColumn.pszText = const_cast<wchar_t*>(col_name.data());

		m_position.push_back((float)len / 100.0f);
		InsertColumn(m_nColumn++, &lvColumn);
	}
	BOOL MoveWindow(
		_In_ int x,
		_In_ int y,
		_In_ int nWidth,
		_In_ int nHeight,
		_In_ BOOL bRepaint = TRUE) noexcept {
		BOOL ok = CListViewCtrl::MoveWindow(x, y, nWidth, nHeight, bRepaint);

		for (int i = 0; i < m_nColumn; ++i)
			SetColumnWidth(i, (int)(m_position[i] * nWidth));

		return ok;
	}
	BOOL MoveWindow(
		_In_ LPCRECT lpRect,
		_In_ BOOL bRepaint = TRUE) noexcept {
		ATLASSERT(::IsWindow(m_hWnd));
		return MoveWindow(lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
	}
private:
	int m_nColumn = 0;
	std::vector<float> m_position;
};

class CListControlView : public CViewItemWindowImpl<CListControlView>
{
	using base = CViewItemWindowImpl<CListControlView>;
public:
	DECLARE_WND_CLASS(NULL)

	CListControlView(CTreeItemAbstract* item, CMyTabView& tabview)
		: base(item, tabview) {
		selected_ = nullptr;
	}

	BEGIN_MSG_MAP(CListControlView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_UPDATE_MYLIST, OnDataChanged)
		NOTIFY_ID_HANDLER(3000, OnListNotify)
		COMMAND_RANGE_HANDLER(0XB000, 0XD000, OnCommand)
	END_MSG_MAP()

	virtual bool IsModified() { return false; };
	virtual void Save() {};
	virtual HICON GetIcon() const noexcept;
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDataChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void UpdateList(bool use_old);

	CMyListViewCtrl list_;
	InteractiveItem* selected_;
	std::unique_ptr<CViewAdapter> view_adapter_;
};


