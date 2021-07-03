#pragma once

#include "dlgbase.h"
#include <npsys/strings.h>
#include "myedit.h"

class CDlg_String
	: public CMyDlgBase<CDlg_String>
{
	using base = CMyDlgBase<CDlg_String>;

	size_t number_;
	npsys::strings_n& strings_;
	CMyEdit edit_name_;
	CMyEditNumber edit_number_;
	CMyButton m_cbOk;
	
public:
	enum { IDD = IDD_DLG_STRING };

	CDlg_String(size_t number, npsys::strings_n& strings)
		: number_(number), strings_(strings)
	{
	}

	BEGIN_MSG_MAP(CDlg_String)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());

		edit_name_.SubclassWindow(GetDlgItem(IDC_EDIT1));
		edit_number_.SubclassWindow(GetDlgItem(IDC_EDIT2));
		m_cbOk.SubclassWindow(GetDlgItem(IDOK));
		
		auto x = strings_->get(number_);
		
		::SetWindowTextA(edit_name_, x->c_str());
		::SetWindowTextA(edit_number_, std::to_string(number_).c_str());
		
		edit_name_.SetFocus();
		edit_name_.SetSelAll();

		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			auto name = GetWindowTextToStdStringW(edit_name_);
			auto number = edit_number_.as<uint32_t>();
			
			if (number_ == number && *strings_->get(number_) == name) {
				// hasn't changed
			} else {
				if (number != number_) {
					for (auto& item : *strings_.get()) {
						if (item.first == number) {
							::MessageBoxA(m_hWnd, ("Same number is already assigned to \"" + item.second + "\"").c_str(), "", MB_ICONWARNING);
							return 0;
						}
					}
				}
				strings_->set(number_, number, name);
			}
			EndDialog(wID);
		} catch (...) {}


		return 0;
	}
};
