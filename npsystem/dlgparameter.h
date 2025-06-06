// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dlgbase.h"
#include <npsys/header.h>
#include <npsys/fbdblock.h>
#include <npsys/common_rpc.hpp>

class CDlg_Parameter : public CMyDlgBase<CDlg_Parameter> {
	using base = CMyDlgBase<CDlg_Parameter>;
public:
	enum { IDD = IDD_DLG_PARAMETER };
	
	BEGIN_MSG_MAP(CDlg_Parameter)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	CDlg_Parameter(npsys::fbd_slot_n& slot)
		: slot_(slot) 
	{
	}

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		CheckDlgButton(IDC_CHECK1, slot_->IsHistoryFlag() ? BST_CHECKED : BST_UNCHECKED);
		if (slot_.is_new_node()) {
			CWindow wnd = GetDlgItem(IDC_CHECK1);
			wnd.EnableWindow(FALSE);
		}
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto history = ::SendMessage(GetDlgItem(IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED;
		bool modified = false;
		
		try {
			if (history == false && slot_->IsHistoryFlag()) {
				// remove from history list
				npsys_rpc->server->History_RemoveParameter(slot_.id());
				slot_->flags_ &= ~npsys::CFBDSlot::F_HISTORY;
				modified = true;
			} else if (history == true && !slot_->IsHistoryFlag()) {
				// add to history list
				npsys_rpc->server->History_AddParameter(slot_.id());
				slot_->flags_ |= npsys::CFBDSlot::F_HISTORY;
				modified = true;
			} else {
				// state hasn't changed
			}
		} catch (nprpc::Exception& ex) {
				std::cerr << "NPRPC Error: " << ex.what() << '\n';
		}

		if (modified && !slot_->is_modified()) {
			EndDialog(IDOK); // save
		}
		
		if (modified && slot_->is_modified()) {
			EndDialog(IDCANCEL); // don't save
		}

		EndDialog(wID);
		return 0;
	}

	npsys::fbd_slot_n& slot_;
};
