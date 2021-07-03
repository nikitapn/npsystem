#pragma once

#include "Stuff.h"

class CDlg_NewController : public CDialogImpl<CDlg_NewController>
{
public:
	CDlg_NewController(std::string& type) :
		type_(type) {}

	enum { IDD = IDD_DLG_NEW_CONTROLLER };
	
	BEGIN_MSG_MAP(CDlg_NewController)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_combo = GetDlgItem(IDC_COMBO1);

		m_combo.AddString(L"atmega8");
		m_combo.AddString(L"atmega8_virtual");
		m_combo.AddString(L"atmega16");
		m_combo.AddString(L"atmega16_virtual");
		/*
		m_combo.AddString("atmega64");
		m_combo.AddString("atmega64_virtual");
		*/
		m_combo.SetCurSel(1);
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int ix = m_combo.GetCurSel();
		wchar_t buffer[256];
		int len = m_combo.GetLBText(ix, buffer);
		type_ = narrow(buffer, len);
		EndDialog(wID);
		return 0;
	}

	std::string& type_;
	CComboBox m_combo;
};
