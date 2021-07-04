// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dockable_index.h"

enum class Dock { Left, Bottom, Right, None };

class CDockableWindow : public CWindow {
protected:
	CWindow wndReflector_ = NULL;
	std::string_view m_windowTitle;
public:
	struct StoredState {
		Dock dock;
		int created;
		int unpinned_cxy;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			ar & dock;
			ar & created;
			ar & unpinned_cxy;
		}
	} m_state;

	inline static std::array<StoredState, 10> dockable_windows;
	inline static std::array<CDockableWindow*, 10> dockable_windows_ptr = {};

	CDockableWindow(_In_opt_ HWND hWnd = NULL) noexcept
		: CWindow(hWnd)
	{
	}

	CDockableWindow(const CDockableWindow&) = delete;
	CDockableWindow& operator =(const CDockableWindow&) = delete;

	CDockableWindow(CDockableWindow&&) = delete;
	CDockableWindow& operator =(CDockableWindow&&) = delete;

	CWindow GetParent() const noexcept {
		return wndReflector_ ? wndReflector_.GetParent() : CWindow::GetParent();
	}

	void SetParent(HWND hWndParent) noexcept {
		if (wndReflector_) {
			wndReflector_.SetParent(hWndParent);
		} else {
			CWindow::SetParent(hWndParent);
		}
	}

	auto GetDock() const noexcept { return m_state.dock; }
	auto GetWindowTitle() const noexcept { return m_windowTitle; }
	virtual void CreateDockableWindow(HWND hWndParent, std::string_view windowTitle) = 0;
};



template<typename T, DockIndex _WindowIndex>
class CDockableWindowT : public CDockableWindow {
public:
	static constexpr auto WindowIndex = static_cast<size_t>(_WindowIndex);
	
	CDockableWindowT(_In_opt_ HWND hWnd = NULL) noexcept
		: CDockableWindow(hWnd)
	{
		m_state = dockable_windows[WindowIndex];
		dockable_windows_ptr[WindowIndex] = this;
	}

	virtual void CreateDockableWindow(HWND hWndParent, std::string_view windowTitle) {
		m_windowTitle = windowTitle;
		static_cast<T*>(this)->Create(hWndParent, rcDefault, nullptr,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	}
};



template <class T, class TBase = CDockableWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CDockableWindowWithReflectorImpl : public CWindowWithReflectorImpl<T, TBase, TWinTraits> {
	using base = CWindowWithReflectorImpl<T, TBase, TWinTraits>;
public:
	HWND Create(
		_In_opt_ HWND hWndParent,
		_In_ _U_RECT rect = NULL,
		_In_opt_z_ LPCTSTR szWindowName = NULL,
		_In_ DWORD dwStyle = 0,
		_In_ DWORD dwExStyle = 0,
		_In_ _U_MENUorID MenuOrID = 0U,
		_In_opt_ LPVOID lpCreateParam = NULL)
	{
		auto hWnd = CWindowWithReflectorImpl<T, TBase, TWinTraits>::Create(hWndParent, 
			rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);

		base::wndReflector_ = base::m_wndReflector;

		return hWnd;
	}
};

