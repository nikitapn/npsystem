#pragma once

#include "dlgbase.h"
#include <npsys/header.h>
#include "io_avr.h"

class CDlg_AvrPinConfig : public CMyDlgBase<CDlg_AvrPinConfig> {
	using base = CMyDlgBase<CDlg_AvrPinConfig>;
public:
	enum { IDD = IDD_DLG_PINCONFIG };
	
	BEGIN_MSG_MAP(CDlg_AvrPinConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	CDlg_AvrPinConfig(npsys::avr_pin_n& pin)
		: pin_(pin) {}
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_combo = GetDlgItem(IDC_COMBO1);
		m_checkInverted = GetDlgItem(IDC_CHECK1);

		int states = pin_->GetAllStates();
		int ix = 0;
		auto pin_purp = pin_->GetPinPurpose();
		int sel = 0;
		if (states & avrinfo::PinPurpose::INPUTPU_PIN) {
			m_tab[ix] = avrinfo::PinPurpose::INPUTPU_PIN;
			if (pin_purp == avrinfo::PinPurpose::INPUTPU_PIN) sel = ix;
			m_combo.AddString(L"Discrete Input Pull-Up");
			ix++;
		}

		if (states & avrinfo::PinPurpose::INPUT_PIN) {
			m_tab[ix] = avrinfo::PinPurpose::INPUT_PIN;
			if (pin_purp == avrinfo::PinPurpose::INPUT_PIN) sel = ix;
			m_combo.AddString(L"Discrete Input");
			ix++;
		}
		
		if (states & avrinfo::PinPurpose::OUTPUT_PIN) {
			m_tab[ix] = avrinfo::PinPurpose::OUTPUT_PIN;
			if (pin_purp == avrinfo::PinPurpose::OUTPUT_PIN) {
				CheckDlgButton(IDC_CHECK1, pin_->IsInverted() ? BST_CHECKED : BST_UNCHECKED);
				m_checkInverted.ShowWindow(SW_SHOW);
				sel = ix;
			}
			m_combo.AddString(L"Discrete Output");
			ix++;
		}
		
		if (states & avrinfo::PinPurpose::ANALOG_PIN) {
			m_tab[ix] = avrinfo::PinPurpose::ANALOG_PIN;
			if (pin_purp == avrinfo::PinPurpose::ANALOG_PIN) sel = ix;
			m_combo.AddString(L"Analog Input");
			ix++;
		}
		
		// auto history = ::SendMessage(m_checkInverted, BM_GETCHECK, 0, 0) == BST_CHECKED;
		m_combo.SetCurSel(sel);
		
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		avrinfo::PinPurpose pinPurp = (avrinfo::PinPurpose)m_tab[m_combo.GetCurSel()];
		auto inverted = (::SendMessage(m_checkInverted, BM_GETCHECK, 0, 0) == BST_CHECKED);
		if (pin_->GetPinPurpose() != pinPurp || pin_->inverted_ != inverted) {
			pin_->SetPinPurpose(pinPurp);
			pin_->inverted_ = inverted;
			pin_.store();
			auto ctrl = pin_->ctrl.fetch();
			ctrl->item->CalcAndUpdateHardwareStatus();
		}
		EndDialog(wID);
		return 0;
	}
	npsys::avr_pin_n& pin_;
	CComboBox m_combo;
	CWindow m_checkInverted;

	int m_tab[10];
};
