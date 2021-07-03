#pragma once

#include "dlgbase.h"
#include "module.h"
#include "block.h"
#include "scintillawindow.h"

class CDlg_BlockFunctionEquation
	: public CMyDlgBase<CDlg_BlockFunctionEquation> {
	using base = CMyDlgBase<CDlg_BlockFunctionEquation>;
public:
	enum { IDD = IDD_DLG_BLOCK_FUNCTION };

	CDlg_BlockFunctionEquation(CBlockFunction* block) 
		: block_(block)
	{
	}

	BEGIN_MSG_MAP(CDlg_BlockFunctionEquation)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		SetWindowText(wide(block_->GetName()).c_str());

		m_cbOk.SubclassWindow(GetDlgItem(IDOK));

		auto rc = CRect{ 20, 25, 500, 65 };
		wnd_.Create(m_hWnd, rc, nullptr, WS_CHILD | WS_CLIPCHILDREN | WS_BORDER, 0);
		wnd_.ShowWindow(SW_SHOW);

		wnd_.sci.SetText(block_->equation_);

		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto text = wnd_.sci.GetText();
		if (text != block_->equation_) {
			block_->equation_ = text;
			block_->top->set_modified();
			EndDialog(IDOK);
		} else {
			EndDialog(IDCANCEL);
		}
		return 0;
	}

	CMyButton m_cbOk;
	CBlockFunction* block_;
	ScintillaWindowWrapper_Function wnd_;
};
