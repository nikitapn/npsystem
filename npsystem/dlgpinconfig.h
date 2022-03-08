// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dlgbase.h"
#include <npsys/header.h>
#include "io_avr.h"

class CDlg_AvrPinConfig : public CMyDlgBase<CDlg_AvrPinConfig> {
	using base = CMyDlgBase<CDlg_AvrPinConfig>;

	npsys::avr_pin_n& pin_;
	CComboBox m_combo;
	CComboBox m_combo_adc_ref_voltage;
	CWindow m_checkInverted;
	int m_tab[10];
public:
	enum { IDD = IDD_DLG_PINCONFIG };
	
	BEGIN_MSG_MAP(CDlg_AvrPinConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnPinPurposeChanged)
	END_MSG_MAP()

	CDlg_AvrPinConfig(npsys::avr_pin_n& pin)
		: pin_(pin) 
	{
	}
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_combo = GetDlgItem(IDC_COMBO1);
		m_combo_adc_ref_voltage = GetDlgItem(IDC_COMBO2);
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
		
		CheckDlgButton(IDC_CHECK1, pin_->IsInverted() ? BST_CHECKED : BST_UNCHECKED);

		if (states & avrinfo::PinPurpose::OUTPUT_PIN) {
			m_tab[ix] = avrinfo::PinPurpose::OUTPUT_PIN;
			if (pin_purp == avrinfo::PinPurpose::OUTPUT_PIN) {
				m_checkInverted.ShowWindow(SW_SHOW);
				sel = ix;
			}
			m_combo.AddString(L"Discrete Output");
			ix++;
		}
		
		if (states & avrinfo::PinPurpose::ANALOG_PIN) {
			m_combo_adc_ref_voltage.AddString(L"AREF, Internal Vref turned off");
			m_combo_adc_ref_voltage.AddString(L"AVCC with external capacitor at AREF pin");
			m_combo_adc_ref_voltage.AddString(L"Internal 2.56V Voltage Reference with external capacitor at AREF pin");
			m_tab[ix] = avrinfo::PinPurpose::ANALOG_PIN;
			if (pin_purp == avrinfo::PinPurpose::ANALOG_PIN) {
				m_combo_adc_ref_voltage.ShowWindow(SW_SHOW);
				sel = ix;
			}
			m_combo.AddString(L"Analog Input");
			ix++;
		}

		m_combo_adc_ref_voltage.SetCurSel(static_cast<int>(pin_->adc_reference_voltage));
		
		// auto history = ::SendMessage(m_checkInverted, BM_GETCHECK, 0, 0) == BST_CHECKED;
		m_combo.SetCurSel(sel);
		
		return 0;
	}

	LRESULT OnPinPurposeChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto pin_purp = static_cast<avrinfo::PinPurpose>(m_tab[m_combo.GetCurSel()]);

		if (pin_purp == avrinfo::PinPurpose::OUTPUT_PIN) {
			m_checkInverted.ShowWindow(SW_SHOW);
		} else {
			m_checkInverted.ShowWindow(SW_HIDE);
		}

		if (pin_purp == avrinfo::PinPurpose::ANALOG_PIN) {
			m_combo_adc_ref_voltage.ShowWindow(SW_SHOW);
		} else {
			m_combo_adc_ref_voltage.ShowWindow(SW_HIDE);
		}

		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		auto pin_purp = static_cast<avrinfo::PinPurpose>(m_tab[m_combo.GetCurSel()]);
		bool analog_pin_reference_voltage_changed = false;
		avrinfo::AdcReferenceVoltage adc_reference_voltage;

		if (pin_purp == avrinfo::PinPurpose::ANALOG_PIN) {
			adc_reference_voltage = static_cast<avrinfo::AdcReferenceVoltage>(m_combo_adc_ref_voltage.GetCurSel());
			analog_pin_reference_voltage_changed = (pin_->adc_reference_voltage != adc_reference_voltage);
		}

		auto inverted = (::SendMessage(m_checkInverted, BM_GETCHECK, 0, 0) == BST_CHECKED);
		
		if (pin_->GetPinPurpose() != pin_purp || pin_->inverted_ != inverted || analog_pin_reference_voltage_changed) {
			pin_->SetPinPurpose(pin_purp);
			pin_->inverted_ = inverted;
			if (pin_purp == avrinfo::PinPurpose::ANALOG_PIN) {
				pin_->adc_reference_voltage = adc_reference_voltage;
			}

			pin_.store();
			auto ctrl = pin_->ctrl.fetch();
			ctrl->item->CalcAndUpdateHardwareStatus();
		}

		EndDialog(wID);
		return 0;
	}
};
