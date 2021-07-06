// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "outputwnd.h"
#include "gdiglobal.h"


int COutputEdit::overflow(int c) {
	max_size_ *= 2;
	char_type* tmp = new char_type[max_size_ + 1];

	auto len = length();
	memcpy(tmp, ptr_, len);
	delete[] ptr_;

	ptr_ = tmp;

#if 0 && defined(_MSC_VER)
	setp(ptr_, ptr_ + len, ptr_ + max_size_);
#else
	setp(ptr_, ptr_ + max_size_);
	pbump(static_cast<int>(len));
#endif
	* pptr() = c;
	pbump(1);

	return c;
}
std::streambuf::pos_type COutputEdit::seekoff(off_type offset,
	std::ios_base::seekdir seek, std::ios_base::openmode mode) {
	saved_size_ = length();
	if (seek == std::ios_base::beg) {
		if (offset < 0 || offset > saved_size_) std::streampos(-1);
#if 0 && defined(_MSC_VER)
		setp(ptr_, ptr_ + offset, ptr_ + max_size_);
#else
		setp(ptr_, ptr_ + max_size_);
		pbump(static_cast<int>(offset));
#endif
		return offset;
	} else if (seek == std::ios_base::end) {
		auto pos = saved_size_ + offset;
		if (offset > 0 || pos < 0) return std::streampos(-1);
#if 0 && defined(_MSC_VER)
		setp(ptr_, ptr_ + pos, ptr_ + max_size_);
#else
		setp(ptr_, ptr_ + max_size_);
		pbump(static_cast<int>(pos));
#endif
		return pos;
	}
	return std::streampos(-1);
}

COutputEdit::COutputEdit(size_t initial_size) noexcept
	: ptr_(nullptr)
	, initial_size_(initial_size)
	, saved_size_(0) {
	clear();
	m_state.dock = Dock::Bottom;
}

COutputEdit::~COutputEdit() {
	if (ptr_) delete[] ptr_;
}

size_t COutputEdit::length() const noexcept {
	auto len = pptr() - pbase();
	return saved_size_ > len ? saved_size_ : len;
}

void COutputEdit::clear() noexcept {
	if (ptr_) delete[] ptr_;
	max_size_ = initial_size_;
	ptr_ = new char_type[initial_size_ + 1];
	setp(ptr_, ptr_ + initial_size_);
}

void COutputEdit::ClearOutput() {
	SetSel(0, -1);
	ReplaceSel(L"");
}

void COutputEdit::ToEnd() {
	auto from = GetFirstVisibleLine();
	auto to = GetLineCount();
	LineScroll(to - from);
	UpdateWindow();
}

LRESULT COutputEdit::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	SetFont(gdiGlobal.fntMonospace);
	bHandled = FALSE;
	return 0;
}

void COutputEdit::Copy() {
	std::cout << "COutputWnd::Copy()" << std::endl;
}

int COutputEdit::sync_impl() {
	if (m_hWnd) {
		auto len = ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
		SetSel(len, len);
		ptr_[length()] = '\0';
		::SendMessageA(this->m_hWnd, EM_REPLACESEL, (WPARAM)0, (LPARAM)ptr_);

		//AppendText(wide({ ptr_, length() }).c_str());
		setp(ptr_, ptr_ + max_size_);
	}
	return 0;
}

int COutputEdit::sync() {
	if (g_main_thread_id != std::this_thread::get_id()) {
		PostMessage(WM_SYNCFROMMAINTHREAD, 0, 0);
		return 0;
	}
	return sync_impl();
}

LRESULT COutputEdit::OnSyncFromMainThread(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return sync_impl();
}

/*
void COutputEdit::Print(const char* text, size_t len, bool add_new_line) {
	AppendText(wide({ text, len }).c_str());
	if (add_new_line) AppendText(L"\r\n");
	ToEnd();
}

void COutputEdit::Print(const std::string_view str) {
	if (m_hWnd == NULL) return;
	Print(str.data(), str.length(), true);
}

void COutputEdit::Print(const char *format, ...) {
	if (m_hWnd == NULL) return;
	int len = 0;
	va_list args;
	va_start(args, format);
	len += _vsnprintf_s(m_buf.get(), buffer_size, buffer_size, format, args);
	va_end(args);
	Print(m_buf.get(), len, true);
}
*/

