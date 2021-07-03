#pragma once

#include "dlgbase.h"
#include "module.h"
#include "tr_item.h"

class CDlg_SegmentValue
	: public CMyDlgBase<CDlg_SegmentValue>
{
	using base = CMyDlgBase<CDlg_SegmentValue>;
public:
	enum { IDD = IDD_DLG_SEGMENTVALUE };

	CDlg_SegmentValue(npsys::CI2CModuleSegmentValue& segment_value, CTreeItemAbstract& item)
		: segment_value_(segment_value)
		, item_(item)
	{
	}

	BEGIN_MSG_MAP(CDlg_SegmentValue)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());

		m_comboType = GetDlgItem(IDC_COMBO1);
		m_editVarName.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_cbOk.SubclassWindow(GetDlgItem(IDOK));

		m_editVarName.SetWindowText(wide(item_.GetName()).c_str());

		m_comboType.AddString(L"u8");
		m_comboType.AddString(L"s8");
		m_comboType.AddString(L"u16");
		m_comboType.AddString(L"s16");
		m_comboType.AddString(L"u32");
		m_comboType.AddString(L"s32");
		m_comboType.AddString(L"real");

		m_comboType.SetCurSel(FromTypeToIndex(segment_value_.GetType()));

		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto var_name = GetWindowTextToStdStringW(m_editVarName);
		segment_value_.set_name(var_name);
		item_.ForceSetName(var_name);
		segment_value_.SetType(
			FromIndexToType(m_comboType.GetCurSel()) | npsys::variable::VQUALITY);
		EndDialog(wID);
		return 0;
	}

	int FromIndexToType(int index) {
		switch (index) {
		case 0: 
			return npsys::variable::VT_BYTE;
		case 1: 
			return npsys::variable::VT_SIGNED_BYTE;
		case 2: 
			return npsys::variable::VT_WORD;
		case 3: 
			return npsys::variable::VT_SIGNED_WORD;
		case 4: 
			return npsys::variable::VT_DWORD;
		case 5: 
			return npsys::variable::VT_SIGNED_DWORD;
		case 6: 
			return npsys::variable::VT_FLOAT;
		default:
			ASSERT(FALSE);
			return npsys::variable::VT_UNDEFINE;
		}
	}

	int FromTypeToIndex(int type) {
		switch (npsys::variable::GetClearType(type)) {
		case npsys::variable::VT_BYTE:
			return 0;
		case npsys::variable::VT_SIGNED_BYTE:
			return 1;
		case npsys::variable::VT_WORD:
			return 2;
		case npsys::variable::VT_SIGNED_WORD:
			return 3;
		case npsys::variable::VT_DWORD:
			return 4;
		case npsys::variable::VT_SIGNED_DWORD:
			return 5;
		case npsys::variable::VT_FLOAT:
			return 6;
		default:
			ASSERT(FALSE);
			return -1;
		}
	}

	npsys::CI2CModuleSegmentValue& segment_value_;
	CTreeItemAbstract& item_;
	CComboBox m_comboType;
	CMyEdit m_editVarName;
	CMyButton m_cbOk;
};
