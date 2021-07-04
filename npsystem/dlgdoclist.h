// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

class CDlg_DocList : public CDialogImpl<CDlg_DocList>
{
public:
	enum { IDD = IDD_DLG_DOCLIST };

	CDlg_DocList(CMyTabView& tabview) 
		: m_tabview(tabview) {}

	BEGIN_MSG_MAP(CDlg_DocList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDYES, OnBnClickedYes)
		COMMAND_ID_HANDLER(IDNO, OnBnClickedNo)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
	bool Modified() {
		bool modified = false;
		func([&modified](auto& tab) { modified = true; });
		return modified;
	}
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_listbox = GetDlgItem(IDC_LIST1);
		func([&](auto& tab) { m_listbox.AddString(tab.text.c_str()); });
		return 0;
	}
	LRESULT OnBnClickedYes(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		func([&](auto& tab) { tab.view->Save(); });
		EndDialog(wID);
		return 0;
	}
	LRESULT OnBnClickedNo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
protected:
	template<typename Pred>
	void func(Pred&& pred) {
		for (auto& tab : m_tabview) {
			if (tab.view->IsModified()) pred(tab);
		}
	}
	CMyTabView& m_tabview;
	CListBox m_listbox;
};

class CDlg_DocList1 : public CDialogImpl<CDlg_DocList>
{
public:
	enum { IDD = IDD_DLG_DOCLIST };

	CDlg_DocList1(CMyTabView::TBDATA& tab)
		: m_tab(tab) {}

	BEGIN_MSG_MAP(CDlg_DocList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDYES, OnBnClickedYes)
		COMMAND_ID_HANDLER(IDNO, OnBnClickedNo)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_listbox = GetDlgItem(IDC_LIST1);
		m_listbox.AddString(m_tab.text.c_str());
		return 0;
	}
	LRESULT OnBnClickedYes(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		m_tab.view->Save();
		EndDialog(wID);
		return 0;
	}
	LRESULT OnBnClickedNo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
protected:
	CMyTabView::TBDATA& m_tab;
	CListBox m_listbox;
};
