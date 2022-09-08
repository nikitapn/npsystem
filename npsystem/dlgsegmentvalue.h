// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

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
		m_comboType.AddString(L"i8");
		m_comboType.AddString(L"u16");
		m_comboType.AddString(L"i16");
		m_comboType.AddString(L"u32");
		m_comboType.AddString(L"i32");
		m_comboType.AddString(L"f32");

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
			FromIndexToType(m_comboType.GetCurSel()) | npsys::nptype::VQUALITY);
		EndDialog(wID);
		return 0;
	}

	int FromIndexToType(int index) {
		switch (index) {
		case 0: return npsys::nptype::NPT_U8;
		case 1: return npsys::nptype::NPT_I8;
		case 2: return npsys::nptype::NPT_U16;
		case 3: return npsys::nptype::NPT_I16;
		case 4: return npsys::nptype::NPT_U32;
		case 5: return npsys::nptype::NPT_I32;
		case 6: return npsys::nptype::NPT_F32;
		default:
			ASSERT(FALSE);
			return npsys::nptype::NPT_UNDEFINE;
		}
	}

	int FromTypeToIndex(int type) {
		switch (npsys::variable::GetClearType(type)) {
		case npsys::nptype::NPT_U8:  return 0;
		case npsys::nptype::NPT_I8:  return 1;
		case npsys::nptype::NPT_U16: return 2;
		case npsys::nptype::NPT_I16: return 3;
		case npsys::nptype::NPT_U32: return 4;
		case npsys::nptype::NPT_I32: return 5;
		case npsys::nptype::NPT_F32: return 6;
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
