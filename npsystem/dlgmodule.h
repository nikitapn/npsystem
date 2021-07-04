// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dlgbase.h"
#include "module.h"

class CDlg_Module 
	: public CMyDlgBase<CDlg_Module>
{
	using base = CMyDlgBase<CDlg_Module>;
public:
	enum { IDD = IDD_DLG_MODULE };

	CDlg_Module(npsys::CI2CModule& mod, const std::string& name) 
		: module_(mod)
		, name_(name) {}

	BEGIN_MSG_MAP(CDlg_Module)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_RANGE_HANDLER(IDC_RADIO_BT_1, IDC_RADIO_BT_3, OnAddressSizeChanged)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		
		SetWindowText(wide(name_).c_str());
		
		m_editAddr.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_editAddr.SetWindowText(wide(to_hex(module_.slave_addr_)).c_str());
		
		m_addSize = module_.addr_size_;
		CheckRadioButton(IDC_RADIO_BT_1, IDC_RADIO_BT_3, IDC_RADIO_BT_1 + m_addSize);
		
		m_cbOk.SubclassWindow(GetDlgItem(IDOK));

		return 0;
	}
	LRESULT OnAddressSizeChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		m_addSize = wID - IDC_RADIO_BT_1;
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			module_.slave_addr_ = m_editAddr.as<uint8_t>();
			module_.addr_size_ = m_addSize;
			EndDialog(wID);
		} catch (...) {}
		return 0;
	}


	const std::string& name_;
	npsys::CI2CModule& module_;
	CMyEditNumber m_editAddr;
	CMyButton m_cbOk;
	int m_addSize;
};
