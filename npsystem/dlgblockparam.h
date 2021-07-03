#pragma once

#include "block.h"
#include "dlgbase.h"
#include "algext.h"
#include "network_ext.h"
#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include "module.h"
#include <npdb/find.h>
#include "exception.h"
#include <npsys/strings.h>
class CDlg_BlockParam
	: public CMyDlgBase<CDlg_BlockParam>
{
	using base = CMyDlgBase<CDlg_BlockParam>;
	using variable = npsys::variable;

	struct Param {
		PARAMETER_TYPE param_type;
		int var_type;
		std::string link;
		bool quality;

		Param() = default;
		Param(PARAMETER_TYPE _param_type, int _var_type, std::string& _link, bool _quality)
			: param_type(_param_type)
			, var_type(_var_type)
			, link(_link)
			, quality(_quality)
		{
		}

		bool operator == (Param& p) {
			if (p.param_type == param_type) {
				if (p.param_type != PARAMETER_TYPE::P_VALUE) {
					return p.link == link;
				} else {
					return p.var_type == var_type && p.quality == quality;
				}
			}
			return false;
		}
		
		bool operator != (Param& p) {
			return !this->operator==(p);
		}
	};

	static constexpr int table[] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 3 };

	CWindow m_checkQuality;

	CEdit m_edit_name;
	CEdit m_edit_link_name;
	CComboBox m_combo_strings;
	CMyButton m_button_ok;
	npsys::strings_l strings_list_;
	CParameter* param_;
	CSlot* slot_;
	CBlockComposition* blocks_;
	npsys::algorithm_n& alg_;
	Param prev;

	std::string m_oldname;
	size_t old_name_index_;

	int _first_time;
	int _iPrevIndex;
	int m_iRadio;

public:
	CDlg_BlockParam(CParameter* param, npsys::algorithm_n& alg)
		: alg_(alg)
		, param_(param)
		, m_iRadio(0)
	{
		slot_ = param->GetSlot();
		blocks_ = alg_->GetBlocks();
		m_oldname = param_->GetName();

		auto slot_type = param_->GetSlot()->GetSlotType();

		prev.param_type = param_->GetParamType();

		if (prev.param_type == PARAMETER_TYPE::P_INTERNAL_REF ||
			prev.param_type == PARAMETER_TYPE::P_EXTERNAL_REF) {
			CReference* pRef = static_cast<CReference*>(slot_type);
			prev.link = pRef->GetLinkName();
		} else {
			prev.var_type = slot_type->DefineTypeR();
		}
	}

	enum { IDD = IDD_DLG_IOBLOCK };

	BEGIN_MSG_MAP(CDlg_BlockParam)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_RANGE_HANDLER(IDC_RADIO_BT_1, IDC_RADIO_BT_10, OnTypeSelect)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnTypeSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		int n = wID - IDC_RADIO_BT_1;
		if (n != m_iRadio) {
			m_iRadio = n;
			if (table[m_iRadio] != 1) {
				CheckDlgButton(IDC_CHECK1, BST_CHECKED);
				m_checkQuality.EnableWindow(FALSE);
				m_edit_link_name.ShowWindow(SW_SHOW);
			} else {
				m_checkQuality.EnableWindow(TRUE);
				m_edit_link_name.SetWindowText(L"");
				m_edit_link_name.ShowWindow(SW_HIDE);
			}
		}
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());
		m_button_ok.SubclassWindow(GetDlgItem(IDOK));
		m_edit_name = GetDlgItem(IDC_EDIT_NAME);
		m_checkQuality = GetDlgItem(IDC_CHECK1);
		m_edit_link_name = GetDlgItem(IDC_EDIT_LINK);
		m_combo_strings = GetDlgItem(IDC_COMBO1);

		_iPrevIndex = SetParam(prev);
		m_oldname = param_->GetName();
		m_edit_name.SetWindowText(wide(m_oldname).c_str());

		auto slot_type = param_->GetSlot()->GetSlotType();
		prev.param_type = param_->GetParamType();

		if (prev.param_type == PARAMETER_TYPE::P_INTERNAL_REF ||
			prev.param_type == PARAMETER_TYPE::P_EXTERNAL_REF) {
			CReference* pRef = static_cast<CReference*>(slot_type);
			prev.link = pRef->GetLinkName();
			m_edit_link_name.SetWindowText(wide(prev.link).c_str());
		} else {
			auto var = slot_type->GetVariable();
			if (var) {
				prev.var_type = var->GetClearType();
			} else {
				assert(false);
			}
		}

		strings_list_.fetch_all_nodes();

		::SendMessageA(m_combo_strings, CB_ADDSTRING, 0, (LPARAM)"not set");
		for (auto& string : strings_list_) {
			::SendMessageA(m_combo_strings, CB_ADDSTRING, 0, (LPARAM)string->get_name().c_str());
		}

		if (slot_->top->strings.is_invalid_id()) {
			m_combo_strings.SetCurSel(0);
		} else {
			auto strings = slot_->top->strings.fetch();
			old_name_index_ = strings_list_.get_index(strings);
			m_combo_strings.SetCurSel(static_cast<int>(old_name_index_ + 1));
		}

		return TRUE;
	}

	int SetParam(Param param) {
		int i = -1;
		
		switch (param.param_type) {
		case PARAMETER_TYPE::P_VALUE:
			switch (variable::GetClearType(param.var_type)) {
			case variable::VT_DISCRETE:				i = 0; break;
			case variable::VT_BYTE:						i = 1; break;
			case variable::VT_SIGNED_BYTE:		i = 2; break;
			case variable::VT_WORD:						i = 3; break;
			case variable::VT_SIGNED_WORD:		i = 4; break;
			case variable::VT_DWORD:					i = 5; break;
			case variable::VT_SIGNED_DWORD:		i = 6; break;
			case variable::VT_FLOAT:					i = 7; break;
			default: ASSERT(FALSE); break;
			}
			break;
		case PARAMETER_TYPE::P_INTERNAL_REF: i = 8; break;
		case PARAMETER_TYPE::P_EXTERNAL_REF: i = 9; break;
		default: ASSERT(FALSE); break;
		}

		ASSERT(i != -1);

		m_iRadio = i;
		CheckRadioButton(IDC_RADIO_BT_1, IDC_RADIO_BT_10, IDC_RADIO_BT_1 + i);
		CheckDlgButton(IDC_CHECK1, param_->GetQuaility() ? BST_CHECKED : BST_UNCHECKED);

		if (table[i] != 1) {
			m_checkQuality.EnableWindow(FALSE);
			m_edit_link_name.ShowWindow(SW_SHOW);
			m_edit_link_name.SetWindowText(wide(param.link).c_str());
		}
		return i;
	}

	Param GetParam(int selected_idx) {
		std::string link;

		bool quality = true;
		
		if (table[selected_idx] != 1) {
			link = GetWindowTextToStdStringW(m_edit_link_name);
		} else {
			quality = (::SendMessage(m_checkQuality, BM_GETCHECK, 0, 0) == BST_CHECKED);
		}
		
		switch (selected_idx) {
		case 0: return { PARAMETER_TYPE::P_VALUE, variable::VT_DISCRETE, link, quality };
		case 1: return { PARAMETER_TYPE::P_VALUE, variable::VT_BYTE, link, quality };
		case 2: return { PARAMETER_TYPE::P_VALUE, variable::VT_SIGNED_BYTE, link, quality };
		case 3: return { PARAMETER_TYPE::P_VALUE, variable::VT_WORD, link, quality };
		case 4: return { PARAMETER_TYPE::P_VALUE, variable::VT_SIGNED_WORD, link, quality };
		case 5: return { PARAMETER_TYPE::P_VALUE, variable::VT_DWORD, link, quality };
		case 6: return { PARAMETER_TYPE::P_VALUE, variable::VT_SIGNED_DWORD, link, quality };
		case 7: return { PARAMETER_TYPE::P_VALUE, variable::VT_FLOAT, link, quality };
		case 8: return { PARAMETER_TYPE::P_INTERNAL_REF, variable::VT_UNDEFINE, link, true };
		case 9: return { PARAMETER_TYPE::P_EXTERNAL_REF, variable::VT_UNDEFINE, link, true };
		default:
			ASSERT(FALSE);
			return {};
		}
	}

	void SetSlotType(CSlotType* pSlotType, PARAMETER_TYPE type) {
		//	CSlotType *poldSlotType, *pnewSlotType;
		//	poldSlotType = slot_->GetSlotType();
		alg_->set_modified();
		param_->SetParamType(type);
		slot_->SetSlotType(pSlotType);
		//	pnewSlotType = slot_->GetSlotType();
		//	if (poldSlotType == pnewSlotType)
		//		return;
		//	blocks_->AddCommand(new ChangeSlotTypeCommand(slot_, param_, type, poldSlotType, pnewSlotType, blocks_));
	}

	int SetInternalRef(CSlot* pRefSlot) {
		CInternalRef* pRef = new CInternalRef(pRefSlot);
		color_m color;
		if (pRef->CheckCycle(color, slot_)) {
			::MessageBoxA(m_hWnd, "There is a cyckle... \"", "Invalid Reference", MB_ICONEXCLAMATION);
			delete pRef;
			return -1;
		}
		SetSlotType(pRef, PARAMETER_TYPE::P_INTERNAL_REF);
		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		enum { name_changed, quality_changed };
		std::bitset<5> changed = {};

		auto name = GetWindowTextToStdStringW(m_edit_name);
		
		if (name != m_oldname) changed.set(name_changed);
		
		if (changed.test(name_changed) && !blocks_->CheckName(name, param_)) {
			::MessageBoxA(m_hWnd, "The name is already exists", "", MB_ICONEXCLAMATION);
			return 0;
		}

		auto cur = GetParam(m_iRadio);
		if (prev.quality != cur.quality) changed.set(quality_changed);

		if (cur != prev) {
			if (cur.param_type == PARAMETER_TYPE::P_VALUE) {
				SetSlotType(new CValue(cur.var_type | (cur.quality ? variable::VQUALITY : 0)), PARAMETER_TYPE::P_VALUE);
			} else if (cur.param_type == PARAMETER_TYPE::P_INTERNAL_REF) {
				auto ix = cur.link.find(L'.', 0);
				if (ix == std::string::npos) {
					::MessageBoxA(m_hWnd, "Bad input", "Invalid Reference", MB_ICONEXCLAMATION);
					return -2;
				}
				const auto block_name = cur.link.substr(0, ix);
				const auto slot_name = cur.link.substr(ix + 1);
				auto slot_link = alg_->FindSlot(block_name, slot_name);
				if (!slot_link) {
					::MessageBoxA(m_hWnd, ("There is no \"" + cur.link + "\" in this algorithm").c_str(), "Invalid Reference", MB_ICONEXCLAMATION);
					return 0;
				}
				SetInternalRef((*slot_link)->e_slot.get());
			} else if (cur.param_type == PARAMETER_TYPE::P_EXTERNAL_REF) {
				if (!IsLinkValid(cur.link)) {
					::MessageBoxA(m_hWnd, "Invalid Link", 0, MB_ICONWARNING);
					return 0;
				}
				auto tok = npsys::CFBDSlot::split_path(cur.link);
				bool result;
				try {
					do {
						if (result = CheckExternalLink(tok)) break;
						if (result = CheckPeripheralLink(tok)) break;
					} while (false);
					if (result == false) {
						::MessageBoxA(m_hWnd, "Link was not found", "Invalid Reference", MB_ICONEXCLAMATION);
						return 0;
					}
				} catch (std::exception& ex) {
					::MessageBoxA(m_hWnd, ex.what(), "Invalid Reference", MB_ICONEXCLAMATION);
					return 0;
				}
			}
		}

		if (changed.any()) alg_->set_modified();
		if (changed.test(name_changed)) param_->SetName(name);
		if (changed.test(quality_changed)) param_->quality_ = cur.quality;

		auto name_index = m_combo_strings.GetCurSel();
		if ((old_name_index_ + 1) != name_index) {
			alg_->set_modified();
			slot_->top->set_modified();
			if (name_index == 0) {
				slot_->top->strings = {};
			} else {
				slot_->top->strings = strings_list_[name_index - 1];
			}
		}

		EndDialog(wID);
		return 0;
	}

	bool IsLinkValid(const std::string& stLink) {
		auto len = stLink.length();
		if (len < 4 || stLink[0] != L'/' || stLink[len - 1] == L'/') return false;
		auto i = stLink.find(L'/', 1);
		return i != std::string::npos && i != len - 1 && stLink.find("//") == std::string::npos;
	}

	bool CheckExternalLink(const std::vector<std::string>& tok) {
		if (tok.size() < 3) throw std::runtime_error("Invalid link");

		const std::string& cat_name = tok[0];
		const std::string& alg_name = tok[1];
		const std::string& slot_full_name = tok[2];

		npsys::categories_n categories;
		ASSERT_FETCH(categories);

		auto cat = odb::utils::find_by_name(categories->alg_cats, cat_name);
		if (!cat) return false;

		auto alg_o = odb::utils::find_by_name((*cat)->algs, alg_name);
		if (!alg_o) {
			throw std::runtime_error("There is no such algoithm \"" + alg_name + '\"');
		}

		auto ix = slot_full_name.find(L'.', 0);

		if (ix == std::string::npos) {
			throw std::runtime_error("Expected . after \"" + slot_full_name + '\"');
		}

		const auto block_name = slot_full_name.substr(0, ix);
		const auto slot_name = slot_full_name.substr(ix + 1);

		auto alg = std::move(alg_o.value());

		if (alg == alg_) {
			auto ref_slot = alg->FindSlot(block_name, slot_name);
			if (!ref_slot) {
				throw std::runtime_error("There is no such parameter \"" + slot_full_name + '\"' + " in \"" + alg_name + '\"');
			}
			SetInternalRef((*ref_slot)->e_slot.get());
		} else {
			auto ref_slot = alg->FindSlot(block_name, slot_name);

			if (!ref_slot) {
				throw std::runtime_error("There is no such parameter \"" + slot_full_name + '\"' + " in \"" + alg_name + '\"');
			}

			auto ext_ref = new CExternalReference(ref_slot.value(), alg, alg_);

			color_m color;
			if (ext_ref->CheckCycle(color, slot_)) {
				delete ext_ref;
				throw std::runtime_error("There is a cyckle...");
			}

			SetSlotType(ext_ref, PARAMETER_TYPE::P_EXTERNAL_REF);
		}

		return true;
	}

	bool CheckPeripheralLink(const std::vector<std::string>& tok) {
		if (tok.size() < 4) throw std::runtime_error("Invalid link"); 

		const auto& ctrl_name = tok[0];
		const auto& l1 = tok[1];

		npsys::controllers_l controllers;
		auto ctrl = odb::utils::find_by_name(controllers, ctrl_name);

		if (!ctrl) return false;
		
		const auto link_type = (param_->GetDirection() == CParameter::INPUT_PARAMETER ? npsys::remote::Read : npsys::remote::Write);
		
		if (l1 == "IO") {
			const auto& port_name = tok[2];
			const auto& pin_name = tok[3];
			auto pin_ref = (*ctrl)->CreatePinReference(
				link_type, port_name, pin_name, alg_);
			pin_ref->LoadLink();
			SetSlotType(pin_ref.release(), PARAMETER_TYPE::P_EXTERNAL_REF);
		} else if (l1 == "I2C") {
			if (tok.size() < 5) throw std::runtime_error("Invalid link"); 
			const auto& module_name = tok[2];
			const auto segment_name = tok[3];
			const auto value_name =  tok[4];
			
			auto mlist = (*ctrl)->GetAssignedModules();
			if (!mlist) throw std::runtime_error("Controller does not support I2C");
			
			auto mod = odb::utils::find_by_name((*mlist), module_name);
			if (!mod) throw std::runtime_error("Module does not exist in the controller");
			
			npsys::CI2CModuleSegment* segment = nullptr;
			(*mod)->childs.fetch();
			for (auto& s : (*mod)->childs) {
				if (s->get_name() == segment_name) {
					segment = s.get();
					break;
				}
			}
			if (!segment)  throw std::runtime_error("Segment does not exist in the module");

			npsys::CI2CModuleSegmentValue* value = nullptr;
			for (auto& v : segment->values) {
				if (v->get_name() == value_name) {
					value = v.get();
					break;
				}
			}
			if (!value)  throw std::runtime_error("Value does not exist in the segment");

			auto ref = new CModuleValue(*mod, value->GetId(), link_type);
			ref->LoadLink();
			SetSlotType(ref, PARAMETER_TYPE::P_EXTERNAL_REF);
		} else {
			throw std::runtime_error("Unknown token \"" + l1 + "\". Valid tokens: IO, I2C");
		}

		return true;
	}
};