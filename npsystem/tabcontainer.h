// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <bitset>
#include "gdiglobal.h"
#include "wm_user.h"
#include "dockable.h"


class CMySplitter : public CSplitterWindowImpl<CMySplitter> {
	bool m_bUpdateRelativePosition = true;
	int m_iRelativePosition = 0;
	int m_iMinXY = 0;

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		if(wParam != SIZE_MINIMIZED) SetSplitterRect();
		return 1;
	}
public:
	DECLARE_WND_CLASS_EX(L"CMySplitter", CS_DBLCLKS, COLOR_WINDOW)

	CMySplitter(bool vertical, bool right_or_bottom) 
		: CSplitterWindowImpl<CMySplitter>(vertical)
	{
		m_dwExtendedStyle &= ~SPLIT_PROPORTIONAL;
		if (right_or_bottom) {
			m_dwExtendedStyle |= SPLIT_RIGHTALIGNED;
		}
	}

	BEGIN_MSG_MAP(CMyTabContainer)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CSplitterWindowImpl<CMySplitter>)
	END_MSG_MAP()

	int GetSplitterRelativePos() const noexcept {
		return m_iRelativePosition;
	}

	void SetSplitterRelativePos(int xyPos, bool bUpdate = true) {
		ATLASSERT(xyPos >= 0);

		m_iRelativePosition = xyPos;
		
		UpdateRelativePosition();

		if(bUpdate) UpdateSplitterLayout();
	}

	void SetSplitterMinPosition(int minXY) {
		m_iMinXY = minXY;
	}

	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		if(xyPos == -1)   // -1 == default position
		{
			if(m_bProportionalDefPos)
			{
				ATLASSERT((m_xySplitterDefPos >= 0) && (m_xySplitterDefPos <= m_nPropMax));

				if(m_bVertical)
					xyPos = ::MulDiv(m_xySplitterDefPos, m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge, m_nPropMax);
				else
					xyPos = ::MulDiv(m_xySplitterDefPos, m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge, m_nPropMax);
			}
			else if(m_xySplitterDefPos != -1)
			{
				xyPos = m_xySplitterDefPos;
			}
			else   // not set, use middle position
			{
				if(m_bVertical)
					xyPos = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) / 2;
				else
					xyPos = (m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge) / 2;
			}
		}

		// Adjust if out of valid range
		/*
		int cxyMax = 0;
		if(m_bVertical)
			cxyMax = m_rcSplitter.right - m_rcSplitter.left;
		else
			cxyMax = m_rcSplitter.bottom - m_rcSplitter.top;
		
		if(xyPos < m_cxyMin + m_cxyBarEdge)
			xyPos = m_cxyMin;
		else if(xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
			xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;
		*/
		
		// Set new position and update if requested
		
		if (CheckLimits(xyPos) == false) return 0;

		bool bRet = (m_xySplitterPos != xyPos);
		m_xySplitterPos = xyPos;
		
		if(m_bUpdateRelativePosition) {
			//if (IsInteractive())
			StoreRelativePosition();
		} else {
			m_bUpdateRelativePosition = true;
		}

		if (bUpdate && bRet) {
			UpdateSplitterLayout();
		}

		return bRet;
	}

	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true) {
		if(lpRect == NULL) {
			GetClientRect(&m_rcSplitter);
		} else {
			m_rcSplitter = *lpRect;
		}

		UpdateRelativePosition();

		if(bUpdate) UpdateSplitterLayout();
	}

	bool CheckLimits(int xyPos) {
		if (m_dwExtendedStyle & SPLIT_RIGHTALIGNED) {
			if (m_bVertical) {
				return m_rcSplitter.right - xyPos >= m_iMinXY;
			} else {
				return m_rcSplitter.bottom - xyPos >= m_iMinXY;
			}
		} else {
			return xyPos >= m_iMinXY;
		}
	}

	void UpdateRelativePosition() {
		if (m_dwExtendedStyle & SPLIT_RIGHTALIGNED) {
			if (m_bVertical) {
				m_xySplitterPos = m_rcSplitter.right - m_iRelativePosition;
			} else {
				m_xySplitterPos = m_rcSplitter.bottom - m_iRelativePosition;
			}
		} else {
			m_xySplitterPos = m_iRelativePosition;
		}
	}

	void StoreRelativePosition() {
		m_bUpdateRelativePosition = false;
		if (m_dwExtendedStyle & SPLIT_RIGHTALIGNED) {
			if (m_bVertical) {
				m_iRelativePosition = m_rcSplitter.right - m_xySplitterPos;
			} else {
				m_iRelativePosition = m_rcSplitter.bottom - m_xySplitterPos;
			}
		} else {
			m_iRelativePosition = m_xySplitterPos;
		}
	}
};

class CMySplitterBottom : public CMySplitter {
public:
	CMySplitterBottom() 
		: CMySplitter(false, true) {}
};

class CMySplitterLeft : public CMySplitter {
public:
	CMySplitterLeft() 
		: CMySplitter(true, false) {}
};

class CMySplitterRight : public CMySplitter {
public:
	CMySplitterRight() 
		: CMySplitter(true, true) {}
};

class CMyTabContainerImpl;

class CMyTabContainer {
	struct deleter {
		__declspec(noinline) void operator()(CMyTabContainerImpl* ptr);
	};
	std::unique_ptr<CMyTabContainerImpl, deleter> impl_;
public:
	struct StoredState {
		bool pinned;
		int splitter_cxy_pos;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			ar & pinned;
			ar & splitter_cxy_pos;
		}
	};

	inline static std::array<StoredState, 3> states;
	inline static std::array<CMyTabContainer*, 3> states_ptr;

	CMyTabContainer(Dock dock);
	HWND Create(CMySplitter& splitter);
	void DockWindow(CDockableWindow& wnd, bool show_window = true);
	
	void ActivateWindow(size_t index);
	void CloseWindow(CDockableWindow& wnd);
	void Hide();
	bool IsPinned() const noexcept;
	const StoredState& GetStoredState() const noexcept;
	void UpdateLayoutUnpinned(int cx, int cy);
	operator HWND() const noexcept;
};