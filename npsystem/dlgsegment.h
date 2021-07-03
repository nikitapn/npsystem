#pragma once

#include "dlgbase.h"
#include "module.h"

class CDlg_Segment 
	: public CMyDlgBase<CDlg_Segment> {
	using base = CMyDlgBase<CDlg_Segment>;
public:
	enum { IDD = IDD_DLG_SEGMENT };

	CDlg_Segment(npsys::CI2CModuleSegment& segment, const std::string& segment_name) 
		: segment_(segment)
		, segment_name_(segment_name) {}

	BEGIN_MSG_MAP(CDlg_Segment)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		SetWindowText(wide(segment_name_).c_str());

		m_comboType = GetDlgItem(IDC_COMBO1);
		m_editOffset.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_cbOk.SubclassWindow(GetDlgItem(IDOK));
		m_editOffset.SetWindowText(wide(to_hex(segment_.GetOffset())).c_str());

		m_comboType.AddString(L"Read");
		m_comboType.AddString(L"Write");

		m_comboType.SetCurSel(segment_.GetType());

		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			segment_.SetOffset(m_editOffset.as<uint16_t>());
			segment_.SetType((io::SegmentType)m_comboType.GetCurSel());
			EndDialog(wID);
		} catch (...) {}
		
		return 0;
	}

	npsys::CI2CModuleSegment& segment_;
	const std::string& segment_name_;
	CComboBox m_comboType;
	CMyEditNumber m_editOffset;
	CMyButton m_cbOk;
};
