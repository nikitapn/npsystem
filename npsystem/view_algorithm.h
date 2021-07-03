#pragma once

#include "Graphics.h"
#include "Block.h"
#include "constants.h"
#include "mytabview.h"
#include <npsys/header.h>

class CTreeAlgorithm;

class CAlgorithmView 
	: public CViewItemWindowImpl<CAlgorithmView> {
	using SelectedElements = std::vector<CElement*>;
	using base = CViewItemWindowImpl<CAlgorithmView>;
	enum TimerId {
		TMR_DEBUG_INFO = 1
	};
public:
	DECLARE_WND_CLASS(NULL)
	CAlgorithmView(CTreeAlgorithm* item, CMyTabView& tabview);
	BOOL PreTranslateMessage(MSG* pMsg) {
		pMsg;
		return FALSE;
	}
	BEGIN_MSG_MAP(CAlgorithmView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DATA_UPDATED, OnUpdateOnlineData)
		MESSAGE_HANDLER(WM_CONNECTION_LOSS, OnOnlineConnectionLoss)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PG_ITEMCHANGED, OnPropertyChanged)
		COMMAND_ID_HANDLER(ID_START_ONLINE, OnStartOnline)
		COMMAND_ID_HANDLER(ID_STOP_ONLINE, OnStopOnline)
		COMMAND_ID_HANDLER(ID_UPLOAD_ALGORITHM, OnUploadToDevice)
		COMMAND_ID_HANDLER(ID_ALGORITHM_PROPERTIES, OnAlgorithmProperties)
		COMMAND_ID_HANDLER(ID_BLOCK_PROPERTIES, OnBlockProperties)
		COMMAND_ID_HANDLER(ID_EXPAND_INPUTS, OnBlockExpandInputs)
		COMMAND_ID_HANDLER(ID_CONFIGURABLE_BLOCK_EDIT, OnBlockConfigurableBlockEdit)
		COMMAND_ID_HANDLER(ID_CONFIGURABLE_BLOCK_EQUATION, OnBlockFucntionEquation)
		COMMAND_ID_HANDLER(ID_REMOVE_SLIDER_THING, OnRemoveSliderThing)
		COMMAND_ID_HANDLER(ID_SLIDER_ADD_TIME_POINT, OnAddTimePoint)

		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnUndo)
		COMMAND_ID_HANDLER(ID_EDIT_REDO, OnRedo)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(ID_AE_COPY_AS_LINK, OnCopyAsLink)
		COMMAND_ID_HANDLER(ID_AE_COPY_SLOT_PATH_TO_CB, OnCopySlotPathToClipboard)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnSave)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { return TRUE; }
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUpdateOnlineData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOnlineConnectionLoss(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	 
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPropertyChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnStartOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStopOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUploadToDevice(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBlockProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAlgorithmProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBlockExpandInputs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBlockConfigurableBlockEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBlockFucntionEquation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveSliderThing(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddTimePoint(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyAsLink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopySlotPathToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	// CView
	virtual bool IsModified();
	virtual void Save();
	virtual std::optional<HBRUSH> CustomActiveColor() const noexcept;
	virtual BOOL CanUndo();
	virtual BOOL CanRedo();
	virtual BOOL CanStartOnline(); 
	virtual BOOL CanStopOnline();
	virtual BOOL CanUpload();
public:
	npsys::algorithm_n alg_;
//	static npsys::CAlgorithmExt* GetOpenedAlg(int alg_id);
	void SetCursor(AlphaCursor newcursor) { m_ChangeCursor = newcursor; }
	float CalcNearestPos(float value) const;
	void Insert(CLine* pLine, const CPoint* pt = nullptr);
	void Insert(npsys::fbd_block_n&& block, const CPoint* pt);
	void Insert(std::unique_ptr<CBlockCompositionWrapper> source, const CPoint* pt = nullptr);
	CGraphics* GetGraphics() { return m_graphics.get(); }
	CElement::MyQuadrantNode qtRoot;
private:
	QuadrantAllocator qtPool_;
	AlphaCursor m_ChangeCursor;
	S_CURSOR m_curState;
//	std::atomic_bool m_animation_thread;
//	std::atomic_bool m_animation;
	float m_animation_angle;
	CPoint m_ptCursor;
	CSlot* f_slot;
	CElement* _element;
	D2D1::MyPoint2F pt_cursor_;
	CElement* pointed_element_ = nullptr;
	bool tooltip_was_showed_ = false;
	CManipulator* m_pMpr;
	size_t m_nRectSelected;
	D2D1_POINT_2F m_ptRect;
	D2D1_RECT_F m_selRect;
	CPoint m_ptMove;
	CBlockComposition* editor_;
	std::unique_ptr<CGraphics> m_graphics;
	int render_frame_ = -1;

	SelectedElements m_selElements;
	class CAlgorithmMultiManipulator : public CMultiManipulator {
	public:
		virtual ~CAlgorithmMultiManipulator() = default;
		virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);
		virtual void Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
		virtual void Draw(CGraphics* pGraphics);
	private:
		CFindSelectedBlocks m_selBlocks;
		std::vector<CLine*> m_lines;
	} m_groupManipulator;
	CPoint m_ptMenu;
	CElement* m_prevSelectedOnline;
	CMenu m_menu;
	CMenu m_menu_selected;

	void RenderBlur();
	void RenderMain();
	void SelectCursor(const D2D1_POINT_2F& pt);
	void SelectCursorOnline(const D2D1_POINT_2F& pt);
	void OnOnlineLButtonDown(UINT nFlags, CPoint point);
	void OnOnlineMouseMove(UINT nFlags, CPoint point);
	void AnimationLoop();
	void UnSelectSelected();
	void CreateBlock(WORD wID, const CPoint* pt);
	void FillPropertyGrid(CElement* elem) noexcept;
};

inline float CAlgorithmView::CalcNearestPos(float fltValue) const {
	int value = static_cast<int>(fltValue);
	int p, m, n;
	
	p = value / constants::block::GRIDSZ_L;
	m = value % constants::block::GRIDSZ_L;
	n = p * constants::block::GRIDSZ_L;
	if (m > constants::block::GRIDSZ_L / 2) n += constants::block::GRIDSZ_L;
	return static_cast<float>(n);
}

#include "quadrant.h"