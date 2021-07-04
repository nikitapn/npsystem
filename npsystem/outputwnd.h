// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dockable.h"

class COutputEdit 
	: public CWindowImpl<COutputEdit, CRichEditCtrlT<CDockableWindowT<COutputEdit, DockIndex::Output>>>
	, public CRichEditCommands<COutputEdit>
	, public std::streambuf {
	char_type* ptr_;
	size_t max_size_;
	size_t initial_size_;
	int64_t saved_size_;
protected:
	virtual int overflow(int c) override;
	virtual pos_type seekoff(off_type offset,
		std::ios_base::seekdir seek, 
		std::ios_base::openmode mode);
	virtual int sync();
public:
	explicit COutputEdit(size_t initial_size = 1 * 1024 * 1024) noexcept;
	~COutputEdit();

	HWND Create(
		_In_opt_ HWND hWndParent,
		_In_ _U_RECT rect = NULL,
		_In_opt_z_ LPCTSTR szWindowName = NULL,
		_In_ DWORD dwStyle = 0,
		_In_ DWORD dwExStyle = 0,
		_In_ _U_MENUorID MenuOrID = 0U,
		_In_opt_ LPVOID lpCreateParam = NULL) {

		return __super::Create(hWndParent, rect, nullptr, dwStyle | ES_MULTILINE | WS_VSCROLL, dwExStyle, MenuOrID, lpCreateParam);
	}

	size_t length() const noexcept;
	void clear() noexcept;
	
	BEGIN_MSG_MAP(COutputEdit)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP_ALT(CRichEditCommands<COutputEdit>, 1)
	END_MSG_MAP()
	
	void ClearOutput();
	// overloads
	void Copy();
protected:
	void ToEnd();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};