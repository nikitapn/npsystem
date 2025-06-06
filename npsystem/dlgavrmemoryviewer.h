// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/common_rpc.hpp>

class Dlg_AVRMemoryViewer 
	: public CDialogImpl<Dlg_AVRMemoryViewer> {
	using base = CDialogImpl<Dlg_AVRMemoryViewer>;

	static constexpr auto BACK_COLOR = RGB(200, 200, 230);
	static constexpr auto words_per_line = 8;
	
	static constexpr size_t line_to_index(size_t line) {
		return line * words_per_line;
	}

	oid_t dev_id_;
	int page_size_;
	CFont font_;
	size_t ix_begin_;
	size_t ix_count_;
	int max_scroll_pos_;
	std::vector<uint16_t> flash_;

	static constexpr uint32_t __ch_height = 28;
public:

	enum { IDD = IDD_DLG_AVR_MEMORY_VIEWER };

	Dlg_AVRMemoryViewer(oid_t dev_id, int page_size) 
		: dev_id_(dev_id), page_size_(page_size) {
		npsys_rpc->server->AVR_V_GetFlash(dev_id_, flash_);
	}

	BEGIN_MSG_MAP(Dlg_AVRMemoryViewer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LOGFONT lf = {};
		lf.lfHeight = __ch_height;

		const char fn[] = "Consolas";
		memcpy_s(lf.lfFaceName, LF_FACESIZE * 2, fn, sizeof(fn));

		font_.CreateFontIndirect(&lf);

		CRect rect;
		GetClientRect(&rect);

		size_t flash_size_ = flash_.size();

		size_t max_lines = flash_size_ / words_per_line;
		size_t max_lines_in_wnd = rect.Height() / __ch_height;

		ix_begin_ = 0;
		ix_count_ = line_to_index(max_lines_in_wnd);

		if (max_lines > max_lines_in_wnd) {
			max_scroll_pos_ = int(max_lines - max_lines_in_wnd);
			ShowScrollBar(SB_VERT);
			SetScrollRange(SB_VERT, 0, max_scroll_pos_);
		}
		return 0;
	}
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		WORD nPos = HIWORD(wParam);
		WORD nSBCode = LOWORD(wParam);

		int pos;
		auto set_pos = [&](float mult) {
			pos = GetScrollPos(SB_VERT);
			pos += static_cast<int>(max_scroll_pos_ * mult);
			if (pos < 0)
				pos = 0;
			else if (pos >= max_scroll_pos_)
				pos = max_scroll_pos_;

			SetScrollPos(SB_VERT, pos, true);
		};

		switch (nSBCode) {
		case SB_ENDSCROLL:
			return 0;
		case SB_LINEDOWN:
			set_pos(0.01f);
			break;
		case SB_LINEUP:
			set_pos(-0.01f);
			break;
		case SB_PAGEDOWN:
			set_pos(0.1f);
			break;
		case SB_PAGEUP:
			set_pos(-0.1f);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			SetScrollPos(SB_VERT, nPos, true);
			pos = nPos;
			break;
		case SB_TOP:
			return 0;
		case SB_BOTTOM:
			return 0;
		default:
			return 0;
		}
		ix_begin_ = line_to_index(pos);
		Invalidate();
		return 0;
	}
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CPaintDC dc(m_hWnd);

		CRect rect;
		GetClientRect(&rect);

		CBitmap mem_bm;
		mem_bm.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());

		CDC mem_dc;
		mem_dc.CreateCompatibleDC(dc);
		mem_dc.SelectBitmap(mem_bm);

		mem_dc.FillSolidRect(&rect, BACK_COLOR);
		mem_dc.SetBkColor(BACK_COLOR);

		HFONT oldfont = mem_dc.SelectFont(font_);

		DrawFlash(
			mem_dc,
			RGB(0, 0, 0),
			RGB(50, 120, 120),
			ix_begin_, ix_begin_ + ix_count_, __ch_height);

		mem_dc.SelectFont(oldfont);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), mem_dc, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
		return 0;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		DestroyWindow();
		return 0;
	}
	void DrawFlash(HDC hdc, COLORREF col1, COLORREF col2, size_t from, size_t to, uint32_t ch_height) {
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		uint32_t y = 0;

		auto draw_line = [&]() {
			std::string str(ss.str());
			ss.str(std::string());
			::TextOutA(hdc, 0, y, str.c_str(), static_cast<int>(str.length()));
			y += ch_height;
		};

		COLORREF col_1;
		COLORREF col_2;

		const auto P_SIZE_W = page_size_ / 2;

		if (((from / P_SIZE_W) % 2) == 0) {
			SetTextColor(hdc, col1);
			col_1 = col1;
			col_2 = col2;
		} else
		{
			SetTextColor(hdc, col2);
			col_1 = col2;
			col_2 = col1;
		}

		bool new_page = (from % P_SIZE_W == 0 ? true : false);

		for (size_t i = from, k = 1; i < to; i++, k++) {
			int w = flash_[static_cast<int>(i)];
			if ((i % P_SIZE_W) == 0 && i != from) {
				std::swap(col_1, col_2);
				::SetTextColor(hdc, col_1);
				new_page = true;
			}

			if (k == 8) {
				k = 0;
				ss << std::setw(4) << w;
				if (new_page) {
					auto page = i / P_SIZE_W;
					ss << "  " << std::dec << page << std::hex;
					new_page = false;
				}
				draw_line();
			} else if (k % 4 == 0) {
				ss << std::setw(4) << w << " | ";
			} else
			{
				if (k == 1) {
					ss << std::setw(4) << i * 2 << "   " << std::setw(4) << w << " ";
				} else
				{
					ss << std::setw(4) << w << " ";
				}
			}
		}
	}
};
