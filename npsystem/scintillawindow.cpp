#include "stdafx.h"
#include "scintillawindow.h"

// ASM
static const char AVR_Instructions[] =
"add adc sub subi sbc sbci and andi or ori eor com neg sbr cbr inc dec tst clr ser adiw sbiw mul muls mulsu"
" rjmp rcall ret reti cpse cp CItemServer cpi sbrc sbrs sbic sbis brbs brbc breq brne brcs brcc brsh brlo brmi brpl brge brlt brhs brhc brtc brvc brie brid ijmp icall jmp call"
" ld st mov ldi in out lpm spm lds sts push pop mowv elpm"
" sbi cbi lsl lsr rol ror asr swap bset bclr bst bld sec clc sen cln sez clz sei cli ses cls sev clv set clt seh clh nop sleep wdr";
static const char AVR_Registers[] =
"r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15"
" r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 r29 r30";
static const char AVR_Directives[] =
" .byte .cseg .csegsize .db .def .dseg .dw .endm .endmacro .equ .eseg .exit .include .list .listmac .macro .nolist .org .set .else .elif .endif .error .if .ifdef .ifndef"
" .message .dd .dq .undef .warning .overlap .nooverlap";
// C
static const char C_Keywords[] =
// Standard
"asm auto bool break case catch char class const "
"const_cast continue default delete do double "
"dynamic_cast else enum explicit extern false finally "
"float for friend goto if inline int long mutable "
"namespace new operator private protected public "
"register reinterpret_cast register return short signed "
"sizeof static static_cast struct switch template "
"this throw true try typedef typeid typename "
"union unsigned using virtual void volatile "
"char while "
// Extended
"__asm __asume __based __box __cdecl __declspec "
"__delegate delegate depreciated dllexport dllimport "
"event __event __except __fastcall __finally __forceinline "
"__int8 __int16 __int32 __int64 __int128 __interface "
"interface __leave naked noinline __noop noreturn "
"nothrow novtable nullptr safecast __stdcall "
"__try __except __finally __unaligned uuid __uuidof "
"__virtual_inheritance";

static unsigned C_color[][2]
{
	{ SCE_C_COMMENT,		RGB(108,213,189) },
	{ SCE_C_COMMENTLINE,	RGB(52,108,151) },
	{ SCE_C_COMMENTDOC,		RGB(52,164,180) },
	{ SCE_C_NUMBER,			RGB(27,205,89) },
	{ SCE_C_STRING,			RGB(223,117,30) },
	{ SCE_C_CHARACTER,		RGB(228,24,126) },
	{ SCE_C_OPERATOR,		RGB(114,115,139) },
	{ SCE_C_PREPROCESSOR,	RGB(80,126,150) },
	{ SCE_C_WORD,			RGB(17,104,236) },
	{ 0,0 }
};

void CScintillaWindow::AutomaticIndentation(char ch) {
	Sci_CharacterRange crange = GetSelection();
	int selStart = static_cast<int>(crange.cpMin);
	int curLine = GetCurrentLineNumber();
	int thisLineStart = (int)Call(SCI_POSITIONFROMLINE, curLine);
	int indentSize = (int)Call(SCI_GETINDENT);
	int indentBlock = IndentOfBlock(curLine - 1);

	if ((ch == '\r' || ch == '\n') && (selStart == thisLineStart)) {
		SetLineIndentation(curLine, indentBlock);
	}
}

int CScintillaWindow::IndentOfBlock(int line) {
	if (line < 0) return 0;
	
	LRESULT indentSize = Call(SCI_GETINDENT);

	int indentBlock = GetLineIndentation(line);
	int backLine = line;
	IndentationStatus indentState = isNone;
	if (statementIndent.IsEmpty() && blockStart.IsEmpty() && blockEnd.IsEmpty())
		indentState = isBlockStart;	// Don't bother searching backwards
		/*
		int lineLimit = line - statementLookback;
		if (lineLimit < 0)
		lineLimit = 0;
		while ((backLine >= lineLimit) && (indentState == 0)) {
		indentState = GetIndentState(backLine);
		if (indentState != 0) {
									indentBlock = GetLineIndentation(backLine);
									if (indentState == isBlockStart) {
									if (!indentOpening)
									indentBlock += indentSize;
									}
									if (indentState == isBlockEnd) {
									if (indentClosing)
									indentBlock -= indentSize;
									if (indentBlock < 0)
									indentBlock = 0;
									}
									if ((indentState == isKeyWordStart) && (backLine == line))
									indentBlock += indentSize;
									}
									backLine--;
									}
									*/
	return indentBlock;
}

void CScintillaWindow::SetLineIndentation(int line, int indent) {
	if (indent < 0)
		return;
	Sci_CharacterRange crange = GetSelection();
	Sci_CharacterRange crangeStart = crange;
	int posBefore = GetLineIndentPosition(line);
	Call(SCI_SETLINEINDENTATION, line, indent);
	int posAfter = GetLineIndentPosition(line);
	int posDifference = posAfter - posBefore;
	if (posAfter > posBefore) {
		// Move selection on
		if (crange.cpMin >= posBefore) {
			crange.cpMin += posDifference;
		}
		if (crange.cpMax >= posBefore) {
			crange.cpMax += posDifference;
		}
	} else if (posAfter < posBefore) {
		// Move selection back
		if (crange.cpMin >= posAfter) {
			if (crange.cpMin >= posBefore)
				crange.cpMin += posDifference;
			else
				crange.cpMin = posAfter;
		}
		if (crange.cpMax >= posAfter) {
			if (crange.cpMax >= posBefore)
				crange.cpMax += posDifference;
			else
				crange.cpMax = posAfter;
		}
	}
	if ((crangeStart.cpMin != crange.cpMin) || (crangeStart.cpMax != crange.cpMax)) {
		SetSelection(static_cast<int>(crange.cpMin), static_cast<int>(crange.cpMax));
	}
}

void CScintillaWindow::SetLexer(lexer lex) {
	switch (lex) {
	case CScintillaWindow::ASM:
		Call(SCI_SETLEXER, SCLEX_ASM, 0);
		Call(SCI_SETKEYWORDS, 0, (LPARAM)AVR_Instructions);
		Call(SCI_SETKEYWORDS, 2, (LPARAM)AVR_Registers);
		Call(SCI_SETKEYWORDS, 3, (LPARAM)AVR_Directives);

		Call(SCI_STYLESETFORE, SCE_ASM_COMMENT, RGB(80, 180, 80));
		Call(SCI_STYLESETFORE, SCE_ASM_CPUINSTRUCTION, RGB(35, 34, 244));
		Call(SCI_STYLESETFORE, SCE_ASM_REGISTER, RGB(180, 60, 40));
		Call(SCI_STYLESETFORE, SCE_ASM_DIRECTIVE, RGB(137, 22, 112));
		m_lex = lex;
		break;
	case CScintillaWindow::C:
		Call(SCI_SETLEXER, SCLEX_CPP, 0);
		Call(SCI_SETKEYWORDS, 0, (LPARAM)C_Keywords);
		for (int i = 0; *C_color[i]; i++)
			Call(SCI_STYLESETFORE, C_color[i][0], C_color[i][1]);
		m_lex = lex;
		break;
	case CScintillaWindow::NONE:
		Call(SCI_SETLEXER, 0, 0);
		m_lex = lex;
		break;
	default:
		break;
	}
}

	/*
	LRESULT lRes;
	//font
	Call(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
	Call(SCI_STYLESETSIZE, STYLE_DEFAULT, 14);
	//strings
	Call(SCI_STYLESETFORE, 3, RGB(166, 96, 13));
	//lexer
	SetLexer(lexer::NONE);
	//tab
	Call(SCI_SETTABWIDTH, 4, 0);
	//margin
	Call(SCI_SETMARGINTYPEN, SC_MARGIN_NUMBER, 1);
	Call(SCI_STYLESETFORE, STYLE_LINENUMBER, RGB(50, 50, 180));
	lRes = Call(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"99999");
	Call(SCI_SETMARGINWIDTHN, SC_MARGIN_NUMBER, lRes);
	//
	Call(SCI_SETEOLMODE, SC_EOL_LF);
	//	
	Call(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT);
	*/


/*
LRESULT CScintillaWindow::OnNotify(LPNMHDR lpnmhdr) {
	SCNotification* notification;
	if (lpnmhdr->hwndFrom == m_hWnd) {
		switch (lpnmhdr->code) {
		case SCN_CHARADDED:
			notification = (SCNotification*)lpnmhdr;
			AutomaticIndentation(notification->ch);
			return TRUE;
		case SCN_MODIFIED:
			break;
		default:
			break;
		};
	}
	return FALSE;
}
*/