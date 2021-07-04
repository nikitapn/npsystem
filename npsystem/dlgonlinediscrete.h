// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "mybutton.h"
#include "algext.h"
#include "error.h"
#include "dlgbase.h"
#include <npsys/corba.h>

class CDlg_OnlineDiscrete : public CMyDlgBase<CDlg_OnlineDiscrete> {
	using base = CMyDlgBase<CDlg_OnlineDiscrete>;
public:
	CDlg_OnlineDiscrete(CParameter* parameter, npsys::CAlgorithmExt* alg)
		: parameter_(parameter)
		, alg_(alg) {
		var_ = parameter_->GetSlot()->GetVariable();
	}

	enum { IDD = IDD_DLG_ONLINE_DISCRETE };
	
	BEGIN_MSG_MAP(CDlg_OnlineDiscrete)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDC_RADIO_BT_FALSE, OnRadioFalse)
		COMMAND_ID_HANDLER(IDC_RADIO_BT_TRUE, OnRadioTrue)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		CenterWindow(GetParent());
		
		m_button_ok.SubclassWindow(GetDlgItem(IDOK));
		m_status = GetDlgItem(IDC_COMBO1);

		auto slot = parameter_->GetSlot();
		const auto& value = slot->OnlineGetValue();
		try {
			value_ = var_->IsQuality() ? value.to_Qbit().value : value.to_bit().value;
			CheckRadioButton(IDC_RADIO_BT_FALSE, IDC_RADIO_BT_TRUE, IDC_RADIO_BT_FALSE + value_);
			if (var_->IsQuality()) {
				m_status.AddString(L"Good quality");
				m_status.AddString(L"Bad quality");
				m_status.SetCurSel(value.to_Qbit().quality == false);
			} else {
				m_status.ShowWindow(SW_HIDE);
			}
		} catch (std::exception& ex) {
			::MessageBoxA(g_hMainWnd, "Error", ex.what(), MB_ICONERROR);
			return TRUE;
		}
		SetWindowText(wide(parameter_->GetName()).c_str());
		return 0;
	}

	LRESULT OnRadioTrue(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		value_ = true;
		return 0;
	}

	LRESULT OnRadioFalse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		value_ = false;
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			if (var_->IsQuality()) {
				npsys_rpc->server->Write_q1(
					var_->GetDevAddr(),
					var_->GetAddr(),
					var_->GetBit(),
					(uint8_t)value_,
					m_status.GetCurSel() == 0 ? uint8_t(0xFF) : uint8_t(0x00)
				);
			} else {
				npsys_rpc->server->Write_1(
					var_->GetDevAddr(),
					var_->GetAddr(),
					var_->GetBit(),
					(uint8_t)value_
				);
			}
		} catch (nps::NPNetCommunicationError& ex) {
			std::cerr << "Communication failed: " << TranslateError(ex.code).data() << '\n';
		} catch (nprpc::Exception& ex) {
			std::cerr << "NPRPC Error: " << ex.what() << '\n';
		}
		EndDialog(wID);
		return 0;
	}

	CParameter* parameter_;
	npsys::CAlgorithmExt* alg_;
	npsys::variable* var_;
	bool value_;

	CComboBox m_status;
	CMyButton m_button_ok;
};
