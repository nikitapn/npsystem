// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nplib/utils/value_parser.h>

template<class T>
class CMyEditT : public CWindowImpl<CMyEditT<T>, CEdit> {
	using base = CWindowImpl<CMyEditT<T>, CEdit>;

	bool m_returnEnabled = false;
public:
	enum NOTIFICATION {
		EN_RETURN = 50200
	};

	DECLARE_WND_SUPERCLASS2(NULL, base, CEdit::GetWndClassName())

	BEGIN_MSG_MAP(CMyEditT)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	BOOL SubclassWindow(_In_ HWND hWnd) {
		BOOL ok = base::SubclassWindow(hWnd);
		if (ok) {
			base::SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		return ok;
	}

	void EnableReturnNotification() { m_returnEnabled = true; }

protected:
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		if (!m_returnEnabled) {
			bHandled = FALSE;
			return 0;
		}
		if (wParam == '\r') {
			SendMessage(base::GetParent(), WM_COMMAND, NOTIFICATION::EN_RETURN << 16 | 0, (LPARAM)base::m_hWnd);
		} else {
			bHandled = FALSE;
		}
		return 0;
	}
};

class CMyEdit : public CMyEditT<CMyEdit> {};

class CMyEditNumber : public CMyEditT<CMyEditNumber> {
public:
	using base = CMyEditT<CMyEditNumber>;

	BEGIN_MSG_MAP(CMyEditNumber)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

	template<typename T>
	void SetValue(T value) { 
		SetWindowText(wide(std::to_string(value)).c_str());
	}

	/*
	void SetValue(float value, int fpart_n = 5) {
		if (fpart_n < 0 || fpart_n > 9) return;
		static constexpr char format[][10]{
			"%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f", "%.7f", "%.8f", "%.9f"
		};
		std::string str;
		str.Format(format[fpart_n], value);
		SetWindowText(str);
	}
	void SetValue(double value, int fpart_n = 5) {
		if (fpart_n < 0 || fpart_n > 9) return;
		static constexpr char format[][10]{
			"%.0lf", "%.1lf", "%.2lf", "%.3lf", "%.4lf", "%.5lf", "%.6lf", "%.7lf", "%.8lf", "%.9lf"
		};
		std::string str;
		str.Format(format[fpart_n], value);
		SetWindowText(str);
	}
	*/

	template<typename T>
	T as() const {
		using types = std::tuple<bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, double>;
		//		static_assert(tuple_contains_type_v<T, types>, "supported types are bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float");
		T value;
		wchar_t buffer[256];
		auto len = GetWindowText(buffer, 256);
		if (len == 0) throw input_error("The value is empty");
		auto text = narrow(buffer, len);
		ParseTextValue(text.c_str(), text.length(), value); 
		::SetFocus(m_hWnd);
		::SendMessage(m_hWnd, EM_SETSEL, 0, -1);
		return value;
	}
protected:
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		char c = (char)wParam;
		if ((c >= '0' && c <= '9') || c == '.' || c == '\b') bHandled = FALSE;
		return 0;
	}
};

class CMyMultilineEdit : public CWindowImpl<CMyMultilineEdit, CEdit> {
	using base = CWindowImpl<CMyMultilineEdit, CMyMultilineEdit>;
public:
	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())

	BEGIN_MSG_MAP(CMyEdit)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()
protected:
	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return DLGC_WANTALLKEYS;
	}
};