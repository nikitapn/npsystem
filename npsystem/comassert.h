#pragma once

#include <atlbase.h>

#if defined(DEBUG) || defined(_DEBUG)
	#define HR(expression) _ASSERTE(S_OK == (expression))
#else
	class ComException : public std::exception
	{
	public:
		ComException(HRESULT hr) : m_value(hr) {}
		virtual const char* what() const override
		{
			static char s_str[64] = { 0 };
			sprintf_s(s_str, "Failure with HRESULT of %08X", m_value);
			return s_str;
		}
		HRESULT Code() {
			return m_value;
		}
	private:
		HRESULT m_value;
	};

	inline void HR(HRESULT hr) {
		if (hr != S_OK) throw ComException(hr);
	}
#endif