#pragma once

#include "dlgbase.h"
#include "module.h"
#include "block.h"

class CDlg_ConfigurableBlock
	: public CMyDlgBase<CDlg_ConfigurableBlock> {
	using base = CMyDlgBase<CDlg_ConfigurableBlock>;
public:
	enum { IDD = IDD_DLG_CONFIGURABLE_BLOCK };

	CDlg_ConfigurableBlock(CConfigurableBlock* block) 
		: block_(block)
	{
	}

	BEGIN_MSG_MAP(CDlg_ConfigurableBlock)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BUTTON1, OnButtonRight)
		COMMAND_ID_HANDLER(IDC_BUTTON2, OnButtonLeft)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()


	std::vector<npsys::fbd_slot_n> added;
	std::vector<npsys::fbd_slot_n> removed;
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		SetWindowText(wide(block_->GetName()).c_str());
		m_cbRight.SubclassWindow(GetDlgItem(IDC_BUTTON1));
		m_cbLeft.SubclassWindow(GetDlgItem(IDC_BUTTON2));
		m_listLeft.Attach(GetDlgItem(IDC_LIST1));
		m_listRight.Attach(GetDlgItem(IDC_LIST2));
	  m_cbOk.SubclassWindow(GetDlgItem(IDOK));
		
		unsigned ix = 0;
		for (auto& slot : block_->disabled_slots_) {
			ASSERT_FETCH(slot);
			m_listLeft.AddString(wide(slot->e_slot->GetName()).c_str());
			m_listLeft.SetItemData(ix, reinterpret_cast<DWORD_PTR>(&slot));
			ix++;
		}

		ix = 0;
		for (auto& slot : block_->top->slots) {
			ASSERT_FETCH(slot);
			m_listRight.AddString(wide(slot->e_slot->GetName()).c_str());
			m_listRight.SetItemData(ix, reinterpret_cast<DWORD_PTR>(&slot));
			ix++;
		}
		
		return 0;
	}

	LRESULT OnButtonRight(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto slot = Transfer(m_listLeft, m_listRight);
		if (!slot) return 0;
		
		auto founded = std::find(removed.begin(), removed.end(), *slot);
		if (founded == removed.end()) added.push_back(*slot);
		else removed.erase(founded);
			
		return 0;
	}

	LRESULT OnButtonLeft(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto slot = Transfer(m_listRight, m_listLeft);
		if (!slot) return 0;

		auto founded = std::find(added.begin(), added.end(), *slot);
		if (founded == added.end()) removed.push_back(*slot);
		else added.erase(founded);
		
		return 0;
	}

	npsys::fbd_slot_n* Transfer(CListBox& from, CListBox& to) {
		auto sel = from.GetCurSel();
		if (sel == -1) return nullptr;

		auto slot = (npsys::fbd_slot_n*)from.GetItemData(sel);
		ASSERT(slot);

		from.DeleteString(sel);
		to.AddString(wide((*slot)->e_slot->GetName()).c_str());
		auto ix_to_last = to.GetCount() - 1;
		to.SetItemData(ix_to_last, (DWORD_PTR)slot);
		to.SetCurSel(ix_to_last);

		if (sel != 0) from.SetCurSel(sel - 1);
		else if (from.GetCount() != 0) from.SetCurSel(sel);

		return slot;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
//			segment_.SetOffset(m_editOffset.as<uint16_t>());
//			segment_.SetType((io::SegmentType)m_comboType.GetCurSel());
			EndDialog(wID);
		} catch (...) {}
		
		return 0;
	}

	CMyButton m_cbOk;
	CMyButton m_cbRight;
	CMyButton m_cbLeft;
	CListBox m_listRight;
	CListBox m_listLeft;
	CConfigurableBlock* block_;
};
