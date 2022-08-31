// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "MyEdit.h"
#include "MyButton.h"

class CDlg_AlgorithmProperties : public CDialogImpl<CDlg_AlgorithmProperties>
{
public:
	CDlg_AlgorithmProperties(npsys::fbd_control_unit_n& alg) :
		alg_(alg) {}

	enum { IDD = IDD_DLG_ALGORITHM_PROPERTIES };

	BEGIN_MSG_MAP(CDlg_AlgorithmProperties)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		REFLECT_NOTIFICATIONS_MSG_FILTERED(WM_DRAWITEM)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());

		m_editScanPeriod_.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_buttonOk.SubclassWindow(GetDlgItem(IDOK));

		SetWindowText(wide(alg_->get_name()).c_str());
		m_editScanPeriod_.SetValue(alg_->scan_period);

		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			uint16_t scan_perion = m_editScanPeriod_.as<uint16_t>();
			if (scan_perion != alg_->scan_period) {
				alg_->SetScanPeriodMs(scan_perion);
				alg_.store();
			}
			EndDialog(wID);
		} catch (input_error& e) {
			MessageBoxA(m_hWnd, e.what(), "Input Error", MB_ICONERROR);
		}
		return 0;
	}

	npsys::fbd_control_unit_n& alg_;
	CMyButton m_buttonOk;
	CMyEditNumber m_editScanPeriod_;
};
