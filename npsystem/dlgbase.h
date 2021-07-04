// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "gdiglobal.h"
#include "myedit.h"
#include "mybutton.h"

template<class T>
class CMyDlgBase : public CDialogImpl<T>
{
public:
	BEGIN_MSG_MAP(CMyDlgBase<T>)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		REFLECT_NOTIFICATIONS_MSG_FILTERED(WM_DRAWITEM)
	END_MSG_MAP()
protected:
	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return (LRESULT)gdiGlobal.brDialog.m_hBrush;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		CDialogImpl<T>::EndDialog(wID);
		return 0;
	}
};