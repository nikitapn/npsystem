// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dlgbase.h"
#include "error.h"

namespace mctrl::detail {
template<typename T>
concept is_systree_item = requires(T x) {
	x.get_name();
};

template<typename T, typename Pred>
class Dlg_Edit {
	T& value_;
public:
	using type = T;

	const char* label;
	bool upper_case;
	Pred pred;

	Dlg_Edit& operator=(T&& value) noexcept {
		value_ = std::move(value);
		return *this;
	}

	const T& value() const noexcept { return value_; }

	Dlg_Edit(const char* _label, bool _upper_case, T& value, Pred&& _pred)
		: label(_label)
		,	upper_case(_upper_case)
		, value_(value)
		, pred(std::forward<Pred>(_pred))
	{
	}
};

template<is_systree_item T, typename Pred>
class Dlg_Edit<T, Pred> {
	T& item_;
public:
	using type = std::string;

	const char* label;
	bool upper_case;
	
	Pred pred;

	Dlg_Edit& operator=(std::string&& value) noexcept {
		item_.set_name(std::move(value));
		return *this;
	}

	const std::string& value() const noexcept { return item_.get_name(); }

	Dlg_Edit(const char* _label, bool _upper_case, T& item, Pred&& _pred)
		: label(_label)
		,	upper_case(_upper_case)
		, item_(item)
		, pred(std::forward<Pred>(_pred))
	{
	}
};

template<typename T>
struct to_types {
	using type = typename T::type;
};

template<typename EditT>
void CreateLabelAndEdit(CStatic& label, EditT& edit, HWND hWnd, CFontHandle font,
	const CRect& rc, const char* label_str, int id, bool upper_case) {
	edit.Create(hWnd, const_cast<CRect&>(rc), NULL, WS_CHILD | ES_AUTOHSCROLL | (upper_case ? ES_UPPERCASE : 0), WS_EX_CLIENTEDGE, id);
	if (label_str) {
		label.Create(hWnd,
			const_cast<CRect&>(CRect{ rc.left - 150, rc.top + 1, rc.left - 20, rc.bottom + 1 }),
			wide(label_str).c_str(), WS_CHILD | WS_VISIBLE);
		label.SetFont(font);
	}
	edit.SetFont(font);
	edit.ShowWindow(SW_SHOW);
}

template<typename T>
concept is_string = std::is_same_v<typename T::type, std::string>;

template<typename T>
struct to_controls {
	using value_type = typename T::type;

	class Wrapper {
		CStatic label_;
		CMyEditNumber edit_;
	public:
		template<size_t Idx>
		void create(HWND hWnd, CFontHandle font, const CRect& rc, value_type value, const char* label, int id, bool upper_case) {
			CreateLabelAndEdit(label_, edit_, hWnd, font, rc, label, id, upper_case);
			edit_.SetValue(value);
			if constexpr (Idx == 0) {
				edit_.SetSel(0, -1);
			}
		}
		value_type get_value() {
			return edit_.as<value_type>();
		}
	};

	using type = Wrapper;
};

template<is_string T>
struct to_controls<T> {
	using value_type = std::string;

	class Wrapper {
		CStatic label_;
		CMyEdit edit_;
	public:
		template<size_t Idx>
		void create(HWND hWnd, CFontHandle font, const CRect& rc, const std::string& str, const char* label, int id, bool upper_case) {
			CreateLabelAndEdit(label_, edit_, hWnd, font, rc, label, id, upper_case);
			::SetWindowTextA(edit_, str.c_str());
			if constexpr (Idx == 0) {
				edit_.SetSel(0, -1);
			}
		}
		std::string get_value() {
			return GetWindowTextToStdStringA(edit_);
		}
	};

	using type = Wrapper;
};

template<typename T>
using to_types_t = typename to_types<T>::type;

template<typename T>
using to_controls_t = typename to_controls<T>::type;

template<typename... Elements>
class CDlg_SimpleInput
	: public CMyDlgBase<CDlg_SimpleInput<Elements...>>
{
	using base = CMyDlgBase<CDlg_SimpleInput<Elements...>>;

	static constexpr size_t N = sizeof...(Elements);
	const char* const m_text;
	std::tuple<Elements...> m_elements;
	mp11::mp_transform<to_controls_t, std::tuple<Elements...>> m_controls;

	CMyButton m_cbOk;
public:
	BEGIN_MSG_MAP(CDlg_SimpleInput)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

	enum { IDD = IDD_DLG_AVR_MEMORY_VIEWER };

	CDlg_SimpleInput(const char* text, Elements&&... elements)
		: m_text(text)
		, m_elements{ elements... }
	{
	}
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		constexpr int width = 800;
		constexpr int height = N * 100 + 100;

		{
			CRect rc{ 0, 0, width, height };
			this->SetWindowPos(NULL, &rc, 0);
			this->CenterWindow(base::GetParent());
		}

		if (m_text != nullptr) ::SetWindowTextA(*this, m_text);

		LOGFONT lf;
		CFontHandle font = this->GetFont();
		font.GetLogFont(&lf);

		int y_offset = 50;
		const auto eh = std::abs(lf.lfHeight) + 8 + 10;
		constexpr int y_gap = 5;

		mp11::mp_for_each<mp11::mp_iota_c<N>>(
			[&](auto Idx) mutable {
				auto& e = std::get<Idx>(m_controls);
				e.template create<Idx>(*this,
					font,
					{ 200, y_offset + y_gap, 550, y_offset + eh - y_gap },
					std::get<Idx>(m_elements).value(),
					std::get<Idx>(m_elements).label,
					IDC_EDIT1 + Idx,
					std::get<Idx>(m_elements).upper_case
					);
				y_offset += eh;
			});

		auto rc = CRect{ width - 150, height - 100, width - 70, height - 70 };
		m_cbOk.Create(*this, &rc, L"Button", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, IDOK);

		return 0;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		try {
			mp11::mp_transform<to_types_t, std::tuple<Elements...>> new_values;

			mp11::mp_for_each<mp11::mp_iota_c<N>>(
				[&](auto Idx) mutable {
					auto& nv = std::get<Idx>(new_values);
					nv = std::get<Idx>(m_controls).get_value();
					std::get<Idx>(m_elements).pred(nv);
				});

			mp11::mp_for_each<mp11::mp_iota_c<N>>(
				[&](auto Idx) mutable {
					std::get<Idx>(m_elements) = std::move(std::get<Idx>(new_values));
				});

			base::EndDialog(wID);
		} catch (input_error& e) {
			e.msg_box(this->m_hWnd);
		} catch (std::exception& e) {
			MessageBoxA(this->m_hWnd, e.what(), "Error", MB_ICONERROR);
		}

		return 0;
	}
};

template<typename T>
struct no_checking {
	void operator()(T&) {}
};

} // namespace mctrl::detail

namespace mctrl {

template<typename T, typename P = detail::no_checking<std::conditional_t<detail::is_systree_item<T>, std::string, T>>>
auto make_text_edit(const char* label, bool upper_case, T& v, P&& p = {}) {
	return detail::Dlg_Edit<T, P>(label, upper_case, v, std::forward<P>(p));
}

template<typename T, typename P = detail::no_checking<T>>
auto make_numeric_edit(const char* label, T& v, P&& p = {}) {
	return make_text_edit<T, P>(label, false, v, std::forward<P>(p));
}

template<typename... Elements>
auto show_dlg_simple_input(const char* dlg_name = nullptr, Elements&&... elements) {
	return detail::CDlg_SimpleInput<Elements...>(dlg_name, std::forward<Elements>(elements)...).DoModal();
}

}// namespace mctrl
