#pragma once

#include <atlwin.h>

enum IndentationStatus {
	isNone, // no effect on indentation
	isBlockStart,	// indentation block begin such as "{" or VB "function"
	isBlockEnd, // indentation end indicator such as "}" or VB "end"
	isKeyWordStart // Keywords that cause indentation
};

struct StyleAndWords {
	int styleNumber;
	std::string words;
	StyleAndWords() : styleNumber(0) { }
	bool IsEmpty() const { return words.length() == 0; }
	bool IsSingleChar() const { return words.length() == 1; }
};

#define MYSCN_ONCHAR		3000
#define MYSCN_ONKEYDOWN	3001
#define MYSCN_ONKEYUP		3002

class CScintillaWindow : public CWindowImpl<CScintillaWindow, CWindow> {
	typedef int(*scintilla_function) (void*, UINT, WPARAM, LPARAM);
public:
	using base = CWindowImpl<CScintillaWindow, CWindow>;
	DECLARE_WND_SUPERCLASS(NULL, L"Scintilla")

	struct MyNotify {
		NMHDR nmhdr;
		int ch;
		BOOL handled;
	};

	enum lexer { ASM, C, NONE } m_lex = NONE;
	
	BEGIN_MSG_MAP(CScintillaWindow)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	int Create(HWND hParentWnd, UINT uIDC) {
		ASSERT(hParentWnd != NULL);
		ASSERT(uIDC != 0);
		
		m_hWndParent = hParentWnd;
		id_ = uIDC;

		m_hWnd = base::Create(hParentWnd, NULL, L"Scintilla", 
			WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0,
			(HMENU)(UINT64)uIDC, NULL);

		ASSERT(m_hWnd != NULL);

		if (m_hWnd == NULL) return -1;

		sc_fun_ = (scintilla_function)::SendMessage(m_hWnd, SCI_GETDIRECTFUNCTION, 0, 0);
		ptr_ = (void*)::SendMessage(m_hWnd, SCI_GETDIRECTPOINTER, 0, 0);

		return 0;
	}
	
	void SetLineIndentation(int line, int indent);
	void AutomaticIndentation(char ch);
	int IndentOfBlock(int line);

	void SetLexer(lexer lex);

	int GetCurrentLineNumber() {
		return (int)Call(SCI_LINEFROMPOSITION, Call(SCI_GETCURRENTPOS));
	}

	void SetText(const std::string& str) {
		Call(SCI_SETTEXT, 0, (LPARAM)str.c_str());
	}

	int GetLenght() {
		return (int)Call(SCI_GETTEXT, 0, NULL);
	}

	std::string GetText() {
		LRESULT lenght = Call(SCI_GETTEXT, 0, NULL);
		if (!lenght) return {};
		std::string text;
		text.resize(lenght);
		Call(SCI_GETTEXT, lenght, (LPARAM)text.data());
		text.pop_back();
		return text;
	}

	LRESULT Call(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const {
		return sc_fun_(ptr_, Msg, wParam, lParam);
	}

	LRESULT operator()(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const {
		return Call(Msg, wParam, lParam);
	}

	Sci_CharacterRange GetSelection() {
		Sci_CharacterRange crange;
		crange.cpMin = (Sci_PositionCR)Call(SCI_GETSELECTIONSTART);
		crange.cpMax = (Sci_PositionCR)Call(SCI_GETSELECTIONEND);
		return crange;
	}

	int GetLineIndentPosition(int line) {
		return (int)Call(SCI_GETLINEINDENTPOSITION, line);
	}

	int GetLineIndentation(int line) {
		return (int)Call(SCI_GETLINEINDENTATION, line);
	}

	void SetSelection(int anchor, int currentPos) {
		Call(SCI_SETSEL, anchor, currentPos);
	}

	void SetSize(int cx, int cy) {
		::SetWindowPos(m_hWnd, NULL, 0, 0, cx, cy, SWP_SHOWWINDOW);
	}

	LRESULT SendKeyNotify(WPARAM wParam, LPARAM lParam, BOOL& bHandled, UINT code) {
		if (!notify_char_) {
			bHandled = FALSE;
			return 0;
		}

		MyNotify n;
		n.nmhdr.code = code;
		n.nmhdr.hwndFrom = m_hWnd;
		n.nmhdr.idFrom = id_;
		n.ch = static_cast<int>(wParam);
		n.handled = TRUE;

		::SendMessage(m_hWndParent, WM_NOTIFY, (WPARAM)id_, (LPARAM)&n);

		bHandled = n.handled;

		return 0;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return SendKeyNotify(wParam, lParam, bHandled, MYSCN_ONKEYDOWN);
	}
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return SendKeyNotify(wParam, lParam, bHandled, MYSCN_ONKEYUP);
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 	SendKeyNotify(wParam, lParam, bHandled, MYSCN_ONCHAR);
	}

	void EnableNotifyChar(bool enable = true) {
		notify_char_ = enable;
	}
protected:
	void* ptr_;
	scintilla_function sc_fun_;
	UINT id_;
	HWND m_hWndParent;
	bool notify_char_ = false;
	bool indentOpening;
	bool indentClosing;
	bool indentMaintain;
	int statementLookback;
	StyleAndWords statementIndent;
	StyleAndWords statementEnd;
	StyleAndWords blockStart;
	StyleAndWords blockEnd;
};

template<class T, UINT Id>
class CScintillaWindowWrapper : public CWindowImpl<T> {
	using base = CWindowImpl<T>;
	T* This() noexcept { return static_cast<T*>(this); }
public:
	DECLARE_WND_CLASS2(NULL, base);

	BEGIN_MSG_MAP(CScintillaWindowWrapper)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	CScintillaWindow sci;

	void SetUp() noexcept {}
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		auto result = sci.Create(base::m_hWnd, Id);
		ASSERT(result == 0);
		if (result == 0) This()->SetUp();
		return result;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
		sci.SetSize(cx, cy);
		return 0;
	}
};

class ScintillaWindowWrapper_Function
	: public CScintillaWindowWrapper<ScintillaWindowWrapper_Function, 900> {
public:
	using base = CScintillaWindowWrapper<ScintillaWindowWrapper_Function, 900>;

	BEGIN_MSG_MAP(CScintillaWindowWrapper)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

	void SetUp() noexcept {
		static auto const text = "OUT := ";
		
		sci(SCI_SETHSCROLLBAR, 0);

		sci(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
		sci(SCI_STYLESETSIZE, STYLE_DEFAULT, 24);

		sci(SCI_SETEOLMODE, SC_EOL_LF);
		sci(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT);
		
		sci(SCI_SETMARGINTYPEN, 1, SC_MARGIN_TEXT);
		//Call(SCI_SETMARGINS, 8);

		auto lRes = sci(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)text);
		sci(SCI_SETMARGINWIDTHN, SC_MARGIN_NUMBER, lRes);
		sci(SCI_MARGINSETTEXT, 0, (LPARAM)text);

		sci.EnableNotifyChar(true);
	}

	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		auto lpnmhdr = (LPNMHDR)lParam;
		if (lpnmhdr->hwndFrom == sci.m_hWnd) {
			switch (lpnmhdr->code) {
			case MYSCN_ONCHAR:
			{
				auto lpnmhdr = (CScintillaWindow::MyNotify*)lParam;
				if (lpnmhdr->ch != 13) lpnmhdr->handled = FALSE;
				break;
			}
			case MYSCN_ONKEYDOWN:
			{
				auto lpnmhdr = (CScintillaWindow::MyNotify*)lParam;
				if (lpnmhdr->ch != VK_RETURN) lpnmhdr->handled = FALSE;
				break;
			}
			case MYSCN_ONKEYUP:
			{
				auto lpnmhdr = (CScintillaWindow::MyNotify*)lParam;
				if (lpnmhdr->ch != VK_RETURN) lpnmhdr->handled = FALSE;
				break;
			}
			default:
				break;
			}
		}
		return 0;
	}
};