#pragma once

#include "memorymanager.h"
#include "mybutton.h"
#include "algext.h"
#include "error.h"
#include "dlgbase.h"
#include <npsys/memtypes.h>

class CDlg_OnlineValue : public CMyDlgBase<CDlg_OnlineValue> {
	using base = CMyDlgBase<CDlg_OnlineValue>;
public:
	enum { IDD = IDD_DLG_ONLINE };

	CDlg_OnlineValue(CParameter* parameter, npsys::CAlgorithmExt* alg)
		: parameter_(parameter)
		, alg_(alg) {
		var_ = parameter_->GetSlot()->GetVariable();
	}

	BEGIN_MSG_MAP(CDlg_NewController)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		CenterWindow(GetParent());
		m_button_ok.SubclassWindow(GetDlgItem(IDOK));
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_status = GetDlgItem(IDC_COMBO1);

		auto slot = parameter_->GetSlot();
		const auto& value = slot->OnlineGetValue();
	
		std::string caption;
		caption = parameter_->GetName();

		if (var_->IsQuality()) {
			m_status.AddString(L"Good quality");
			m_status.AddString(L"Bad quality");
		} else {
			m_status.ShowWindow(SW_HIDE);
		}

		std::string type;
		std::string str;

		switch (var_->GetClearType()) {
		case npsys::variable::VT_DISCRETE:
			type = "discrete";
			if (var_->IsQuality()) {
				str = std::to_string(static_cast<bool>(value.to_Qbit().value));
				SetQuality(value.to_Qbit().quality);
			} else {
				str = std::to_string(static_cast<bool>(value.to_bit().value));
			}
			break;
		case npsys::variable::VT_BYTE:
			type = "byte";
			
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qu8().value);
				SetQuality(value.to_Qu8().quality);
			} else {
				str = std::to_string(value.to_u8().value);
			}
			
			break;
		case npsys::variable::VT_SIGNED_BYTE:
			type = "signed byte";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qi8().value);
				SetQuality(value.to_Qi8().quality);
			} else {
				str = std::to_string(value.to_i8().value);
			}
			break;
		case npsys::variable::VT_WORD:
			type = "word";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qu16().value);
				SetQuality(value.to_Qu16().quality);
			} else {
				str = std::to_string(value.to_u16().value);
			}
			break;
		case npsys::variable::VT_SIGNED_WORD:
			type = "signed word";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qi16().value);
				SetQuality(value.to_Qi16().quality);
			} else {
				str = std::to_string(value.to_i16().value);
			}
			break;
		case npsys::variable::VT_DWORD:
			type = "dword";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qu32().value);
				SetQuality(value.to_Qu32().quality);
			} else {
				str = std::to_string(value.to_u32().value);
			}
			break;
		case npsys::variable::VT_SIGNED_DWORD:
			type = "signed double word";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qi32().value);
				SetQuality(value.to_Qi32().quality);
			} else {
				str = std::to_string(value.to_i32().value);
			}
			break;
		case npsys::variable::VT_FLOAT:
			type = "float";
			if (var_->IsQuality()) {
				str = std::to_string(value.to_Qflt().value);
				SetQuality(value.to_Qflt().quality);
			} else {
				str = std::to_string(value.to_flt().value);
			}
			break;
		default:
			type = "unknown";
			break;
		}

		caption = "[" + type + "]: " + caption;

		SetWindowTextA(m_hWnd, caption.c_str());
		SetWindowTextA(m_edit.m_hWnd, str.c_str());
		m_edit.SetFocus();
		m_edit.SetSel(0, -1);
		m_brush_dialog.CreateSolidBrush(RGB(255, 255, 255));

		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			bool quality = var_->IsQuality();
			switch (var_->GetClearType()) {
			case npsys::variable::VT_DISCRETE:
			{
				bool val = m_edit.as<bool>();
				if (quality) {
					npsys_rpc->server->Write_q1(var_->GetDevAddr(), var_->GetAddr(), var_->GetBit(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_1(var_->GetDevAddr(), var_->GetAddr(), var_->GetBit(), val);
				}
				break;
			}
			case npsys::variable::VT_BYTE:
			{
				uint8_t val = m_edit.as<uint8_t>();
				if (quality) {
					npsys_rpc->server->Write_q8(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_8(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_SIGNED_BYTE:
			{
				int8_t val = m_edit.as<int8_t>();
				if (quality) {
					npsys_rpc->server->Write_q8(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_8(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_WORD:
			{
				uint16_t val = m_edit.as<uint16_t>();
				if (quality) {
					npsys_rpc->server->Write_q16(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_16(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_SIGNED_WORD:
			{
				int16_t val = m_edit.as<int16_t>();
				if (quality) {
					npsys_rpc->server->Write_q16(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_16(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_DWORD:
			{
				uint32_t val = m_edit.as<uint32_t>();
				if (quality) {
					npsys_rpc->server->Write_q32(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_32(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_SIGNED_DWORD:
			{
				int32_t val = m_edit.as<int32_t>();
				if (quality) {
					npsys_rpc->server->Write_q32(var_->GetDevAddr(), var_->GetAddr(), val, GetQuality());
				} else {
					npsys_rpc->server->Write_32(var_->GetDevAddr(), var_->GetAddr(), val);
				}
				break;
			}
			case npsys::variable::VT_FLOAT:
			{
				float val = m_edit.as<float>();
				if (quality) {
					npsys_rpc->server->Write_q32(var_->GetDevAddr(), var_->GetAddr(), *(uint32_t*)&val, GetQuality());
				} else {
					npsys_rpc->server->Write_32(var_->GetDevAddr(), var_->GetAddr(), *(uint32_t*)&val);
				}
				break;
			}
			default:
				ATLASSERT(FALSE);
				break;
			}
			EndDialog(IDOK);
			return 0;

		} catch (input_error& e) {
			e.msg_box(this->m_hWnd);
			return 0;
		} catch (nps::NPNetCommunicationError& ex) {
			std::cout << "Communication failed: " << TranslateError(ex.code).data() << std::endl;
		} catch (nprpc::Exception& ex) {
			std::cout << "Caught CORBA: " << ex.what() << std::endl;
		}

		EndDialog(IDCANCEL);
		return 0;
	}
	void SetQuality(uint8_t quality) {
		m_status.SetCurSel(quality == 0);
	}
	uint8_t GetQuality() const {
		return (m_status.GetCurSel() == 0 
			? VQ_GOOD
			: VQ_BAD);
	}
	
	CParameter* parameter_;
	npsys::CAlgorithmExt* alg_;
	npsys::variable* var_;
	CBrush m_brush_dialog;

	CMyButton m_button_ok;
	CMyEditNumber m_edit;
	CComboBox m_status;
};
