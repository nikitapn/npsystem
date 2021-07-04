// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "algext.h"
#include "view_algorithm.h"
#include "graphics.h"
#include "command.h"
#include "error.h"
#include "dlgsimpleinput.h"
#include "dlgconfigurableblock.h"
#include "dlgblockfunctionequation.h"
#include "blockpropertyid.h"
#include "tr_alg.h"
#include "line.h"
#include "ribbonstate.h"

#define DEBUG_DRAW 0

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC	((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE	((USHORT) 0x02)
#endif

extern UINT g_clf_blocks;

CAlgorithmView::CAlgorithmView(CTreeAlgorithm* item, CMyTabView& tabview)
	: base(item, tabview)
	, qtPool_{ sizeof(CElement::MyQuadrantNode), CElement::MyQuadrantNode::MAX_ELEMENTS_IN_SQUARE * sizeof(void*) }
	, qtRoot({ -10000, -10000, 10000, 10000 }, qtPool_)
{
	alg_ = item->n_;
	m_ChangeCursor = AlphaCursor::Arrow;
	_element = nullptr;
	m_pMpr = nullptr;
	f_slot = nullptr;
	m_curState = S_CURSOR::A_NOT_SELECTED;
	m_animation_angle = 0.0f;
	//	m_animation_thread = false;
	//	m_animation = false;
	m_selElements.reserve(1024);

	editor_ = alg_->GetBlocks();

	if (!editor_) throw 0;
}

LRESULT CAlgorithmView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	alg_->SetWindow(m_hWnd);
	PasteToQuadrant(editor_, &qtRoot);

	ASSERT_FETCH(alg_->view_position);

	auto mat = D2D1::Matrix3x2F(alg_->view_position->scale, 0, 0,alg_->view_position->scale, alg_->view_position->x_offset, alg_->view_position->y_offset);
	m_graphics.reset(new CGraphics(m_hWnd));
	m_graphics->SetMatrix(mat);

	m_menu.LoadMenu(IDR_MENU_ALG_CONTEXT);
	m_menu_selected.LoadMenu(IDR_MENU_ALG_CONTEXT_SELECTED);

	/*
	for (auto& block : alg_->fbd_blocks) {
		for (auto& slot : block->slots) {
			//slot->block_parent = block;
			//slot->alg = alg_;
			//slot.store();
			std::cout << slot->path() << " repaired..." << std::endl;
		}
	}
	*/
	
	return 0;
}
LRESULT CAlgorithmView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	const D2D1::Matrix3x2F& mat = m_graphics->GetMatrix();
	alg_->view_position->SavePosition(mat._31, mat._32, mat._11);
	if (alg_->view_position->is_modified()) {
		auto new_node = alg_->view_position.is_new_node(); // remove in future
		alg_->view_position.store();
		if (new_node) alg_.store(); // remove in future
	}
	alg_->SetWindow(NULL);
	alg_->Unload();
	return 0;
}
LRESULT CAlgorithmView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UINT width = LOWORD(lParam), height = HIWORD(lParam);
	m_graphics->Resize(width, height);
	return 0;
}

void CAlgorithmView::Insert(npsys::fbd_block_n&& block, const CPoint* pt) {
	ASSERT(pt);
	D2D1::MyPoint2F ptf = D2D1::MyPoint2F((float)pt->x, (float)pt->y) * m_graphics->GetInverseMatrix();
	auto size = block->e_block->GetSize();
	block->e_block->Move(
		CalcNearestPos(ptf.x - size.width / 2.0f),
		CalcNearestPos(ptf.y - size.height / 2.0f)
	);
	editor_->AddCommand(new InsertBlockCommand(std::move(block), editor_, &qtRoot), true);
	Invalidate();
}
void CAlgorithmView::Insert(CLine* pLine, const CPoint* pt) {
	editor_->AddCommand(new InsertLineCommand(pLine, editor_, &qtRoot), true);
	Invalidate();
}

void CAlgorithmView::Insert(std::unique_ptr<CBlockCompositionWrapper> source, const CPoint* pt) {
	editor_->AddCommand(new PasteClipboardCommand(std::move(source), editor_, &qtRoot), true);
	Invalidate();
}
void CAlgorithmView::SelectCursor(const D2D1_POINT_2F& pt) {
	UINT flags;
	auto founded = qtRoot.FindElement(pt.x, pt.y, flags);
	if (founded != nullptr) {
		pointed_element_ = founded;
		m_ChangeCursor = founded->GetCursor();
		bool invalidate = false;
		if (flags & EL_INVALIDATE) invalidate = true;
		if (tooltip_was_showed_) invalidate = true;
		tooltip_was_showed_ = false;
		SetTimer(TMR_DEBUG_INFO, 250);
		if (invalidate) Invalidate();
		//		}
	} else {
		pointed_element_ = nullptr;
		m_ChangeCursor = AlphaCursor::Arrow;
	}
}
void CAlgorithmView::SelectCursorOnline(const D2D1_POINT_2F& pt) {
	UINT flags;
	auto founded = qtRoot.FindElement(pt.x, pt.y, flags);
	if (founded != nullptr) {
		m_ChangeCursor = founded->GetOnlineCursor();
	} else {
		m_ChangeCursor = AlphaCursor::Arrow;
	}
}
LRESULT CAlgorithmView::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (g_ribbonState.cursor_selected) {
		::SetCursor(global.GetCursor(m_ChangeCursor));
	} else {
		if (g_ribbonState.block_selected != -1) {
			::SetCursor(global.GetCursor(AlphaCursor::Drag));
		}
	}
	return 0;
}

void CAlgorithmView::RenderMain() {
	m_graphics->DrawGrid();

#if DEBUG_DRAW
	Iter::PreorderIterator<CElement::MyQuadrantNode*, true> it(&qtRoot);
	for (; !it.IsDone(); it.Next()) {
		D2D1_RECT_F&rc = (D2D1_RECT_F&)(*it)->GetBounds();
		m_graphics->DrawRect(rc, SolidColor::Blue);
	}
#endif

	qtRoot.Draw(m_graphics.get(), QuadrantAdapter<CElement, float>::GetVisibleArea(m_graphics->GetVisibleRect()), render_frame_);
	alg_->DrawOnlineValues(m_graphics.get());

	if (m_pMpr) {
		m_pMpr->Draw(m_graphics.get());
	}

	if (m_curState == A_RECT) {
		m_graphics->DrawRect(m_selRect, SolidColor::Green);
	}

	render_frame_++;
}

void CAlgorithmView::RenderBlur() {
	m_graphics->BeginNewBufferedDraw();
	RenderMain();
	m_graphics->RestoreSurface();
	m_graphics->DrawShadow();
	m_graphics->PostEffectBlur();
	m_graphics->EndDraw();
}

void CAlgorithmView::AnimationLoop() {
	/*	using namespace std::literals::chrono_literals;
		while (m_animation_thread) {
			m_graphics->BeginNewBufferedDraw();
			RenderMain();
			m_graphics->RestoreSurface();
			m_graphics->DrawShadow();
			m_graphics->PostEffectBlur();
			m_animation_angle += 5;
			m_graphics->DrawWaitCircle(m_animation_angle);
			m_graphics->EndDraw();
			std::this_thread::sleep_for(18ms);
		}
		m_animation = false;*/
}

LRESULT CAlgorithmView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPaintDC dc(m_hWnd);

	//	if (m_animation.load(std::memory_order_relaxed)) return 0;

	m_graphics->BeginNewBufferedDraw();
	RenderMain();
	m_graphics->RestoreSurface();
	m_graphics->DrawShadow();
	m_graphics->DrawMainBuffer();
	m_graphics->EndDraw();

	return 0;
}

static char StateTab[][15] = {
	"A_NOT_SELECTED",
	"A_ONE_ELEM",
	"A_GROUP",
	"A_LINE",
	"A_RECT",
	"A_SELECTED",
};

void CAlgorithmView::UnSelectSelected() {
	if (m_selElements.size() == 0) return;
	for (auto sel : m_selElements) {
		sel->SetSelect(FALSE);
	}
	m_selElements.clear();
}

void CAlgorithmView::FillPropertyGrid(CElement* elem) noexcept {
	g_pPropertyGrid->ResetContents();
	if (elem == nullptr) {
		g_pPropertyGrid->AddSection(PRB::ID_SECTION_GENERAL, "General");
		g_pPropertyGrid->AddVariantItem(PRB::ID_SECTION_GENERAL, "Id", alg_->self_node.id(), 0, false);
		g_pPropertyGrid->AddVariantItem(PRB::ID_SECTION_GENERAL, "Scan period", alg_->scan_period, PRB::ID_PR_ALG_SCAN_PERIOD);
	} else {
		g_pPropertyGrid->AddSection(PRB::ID_SECTION_GENERAL, "General");
		g_pPropertyGrid->AddSection(PRB::ID_SECTION_POSITION, "Position");
		elem->FillPropertyGrid(g_pPropertyGrid);
	}
}

LRESULT CAlgorithmView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	UINT flags = static_cast<UINT>(wParam);

	//	if (m_animation.load(std::memory_order_relaxed)) return 0;

	SetFocus();

	if (alg_->IsOnline()) {
		OnOnlineLButtonDown(flags, point);
		return 0;
	}

	if (g_ribbonState.block_selected) {
		CreateBlock(g_ribbonState.block_selected, &point);
		return 0;
	}

	SetCapture();
	int shift = GetKeyState(VK_SHIFT) & 0x8000;
	switch (m_curState) {
	case A_NOT_SELECTED:
	{
		if (!shift)
			UnSelectSelected();

		D2D1::MyPoint2F pt = D2D1::Point2F((FLOAT)point.x, (FLOAT)point.y) * m_graphics->GetInverseMatrix();
		UINT elflags;
		auto founded = qtRoot.FindElement(pt.x, pt.y, elflags);

		FillPropertyGrid(founded);
		if (founded != nullptr) {
			if (!founded->IsSelected()) {
				founded->SetSelect(true);
				m_selElements.push_back(founded);
			}
			if (!shift) {
				m_pMpr = founded->GetManipulator();
				if (!m_pMpr) return 0;
				m_curState = m_pMpr->MouseButtonDown(point, this);
			}
		} else if (!shift) {
			UnSelectSelected();
		}
		size_t nSelected = m_selElements.size();
		//		DBG_PRINT("nSelected: %d\n", nSelected);
		if (nSelected == 0) {
			m_curState = A_RECT;
			m_nRectSelected = 0;
			m_ptRect.x = (FLOAT)point.x;
			m_ptRect.y = (FLOAT)point.y;
			m_ptRect = m_ptRect * m_graphics->GetInverseMatrix();;
			m_selRect = { m_ptRect.x, m_ptRect.y, m_ptRect.x, m_ptRect.y };
			return 0;
		} else if (nSelected == 1 && !shift) {
			m_curState = A_ONE_ELEM;
		} else {
			m_curState = A_SELECTED;
		}
		Invalidate();
		break;
	}
	case A_LINE:
	case A_ONE_ELEM:
		break;
	case A_SELECTED:
	{
		if (shift) {
			m_curState = A_NOT_SELECTED;
			OnLButtonDown(uMsg, wParam, lParam, bHandled);
			return 0;
		}
		D2D1::MyPoint2F pt = D2D1::Point2F((float)point.x, (float)point.y) * m_graphics->GetInverseMatrix();
		UINT elflags;
		auto elem = qtRoot.FindElement(pt.x, pt.y, elflags);
		if (elem == nullptr || !elem->IsSelected()) {
			for (auto p : *editor_) {
				if (p->IsSelected()) {
					p->SetSelect(false);
					if (--m_nRectSelected == 0)
						break;
				}
			}
			m_selElements.clear();
			m_curState = A_NOT_SELECTED;
			Invalidate();
			OnLButtonDown(uMsg, wParam, lParam, bHandled);
		} else {
			m_pMpr = &m_groupManipulator;
			m_groupManipulator.MouseButtonDown(point, this);
			m_curState = A_GROUP;
		}
		break;
	}
	default:
		break;
	}
	return 0;
}

LRESULT CAlgorithmView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	UINT flags = static_cast<UINT>(wParam);
	CPoint point(lParam);

	// ??? the messages keep comming when media player is running ???
	static CPoint old_point;
	
	if (old_point == static_cast<POINT>(point)) return 0;
	old_point = point;

	//	if (m_animation.load(std::memory_order_relaxed)) return 0;

	if (flags & MK_MBUTTON) {
		m_graphics->MoveOrgDxDy(static_cast<float>(point.x - m_ptMove.x), static_cast<float>(point.y - m_ptMove.y));
		m_ptMove = point;
		Invalidate();
		return 0;
	}
	
	if (alg_->IsOnline()) {
		OnOnlineMouseMove(flags, point);
		return 0;
	}
	
	switch (m_curState) {
	case A_GROUP:
	case A_ONE_ELEM:
	case A_LINE:
		ASSERT(m_pMpr);
		m_pMpr->Drag(point, this);
		break;
	case A_RECT: 
	{
		auto pt = D2D1::MyPoint2F(
			static_cast<float>(point.x),
			static_cast<float>(point.y)) * m_graphics->GetInverseMatrix();
		if (m_ptRect.x > pt.x) {
			m_selRect.left = pt.x;
			m_selRect.right = m_ptRect.x;
		} else {
			m_selRect.left = m_ptRect.x;
			m_selRect.right = pt.x;
		}
		if (m_ptRect.y > pt.y) {
			m_selRect.top = pt.y;
			m_selRect.bottom = m_ptRect.y;
		} else {
			m_selRect.top = m_ptRect.y;
			m_selRect.bottom = pt.y;
		}
		for (auto elem : *editor_) {
			int wasSelected = elem->IsSelected();
			if (elem->RectInRect(m_selRect)) {
				if (!wasSelected) {
					elem->SetSelect(true);
					m_nRectSelected++;
				}
			} else {
				if (wasSelected) {
					elem->SetSelect(false);
					m_nRectSelected--;
				}
			}
		}
		Invalidate();
		break;
	}
	default:
		pt_cursor_ = D2D1::MyPoint2F(static_cast<float>(point.x), static_cast<float>(point.y)) * m_graphics->GetInverseMatrix();
		//	if (vw_ && vw_->GetCapture()) {
		//		vw_->OnMouseMove(pt_cursor_);
		//	} else {
		SelectCursor(pt_cursor_);
		//	}
		break;
	}
	return 0;
}

LRESULT CAlgorithmView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	//	if (m_animation.load(std::memory_order_relaxed)) return 0;
	if (alg_->IsOnline()) return 0;
	//	DBG_PRINT("OnLButtonUp() = %d\n", m_curState);
	ReleaseCapture();
	switch (m_curState) {
	case A_ONE_ELEM:
	case A_LINE:
		ASSERT(m_pMpr);
		m_pMpr->MouseButtonUP(point, this);
		m_pMpr = nullptr;
		m_curState = A_NOT_SELECTED;
		break;
	case A_GROUP:
		ASSERT(m_pMpr);
		m_pMpr->MouseButtonUP(point, this);
		m_pMpr = nullptr;
		m_curState = A_SELECTED;
		break;
	case A_RECT:
		//	DBG_PRINT("m_nRectSelected: %d\n", m_nRectSelected);
		if (m_nRectSelected) {
			m_selElements.clear();
			for (auto elem : *editor_) {
				if (elem->IsSelected())
					m_selElements.push_back(elem);
			}
			if (m_selElements.size() == 1) {
				FillPropertyGrid(m_selElements[0]);
			}
			m_curState = A_SELECTED;
		} else
		{
			m_curState = A_NOT_SELECTED;
		}
		Invalidate();
		break;
	default:
		break;
	}
	return 0;
}

void CAlgorithmView::OnOnlineLButtonDown(UINT flags, CPoint point) {
	//	if (m_animation.load(std::memory_order_relaxed)) return;

	UINT elflags;
	D2D1::MyPoint2F pt = D2D1::Point2F((float)point.x, (float)point.y) * m_graphics->GetInverseMatrix();
	auto founded = qtRoot.FindElement(pt.x, pt.y, elflags);
	if (founded == nullptr && m_prevSelectedOnline != nullptr) {
		m_prevSelectedOnline->SetSelect(FALSE);
		m_prevSelectedOnline = nullptr;
		Invalidate();
	} else if (founded != m_prevSelectedOnline) {
		if (m_prevSelectedOnline != nullptr)
			m_prevSelectedOnline->SetSelect(FALSE);

		founded->SetSelect(TRUE);

		m_prevSelectedOnline = founded;

		Invalidate();
	}
}
void CAlgorithmView::OnOnlineMouseMove(UINT nFlags, CPoint point) {
	//	if (m_animation.load(std::memory_order_relaxed)) return;
	D2D1::MyPoint2F pt = D2D1::MyPoint2F(
		static_cast<float>(point.x),
		static_cast<float>(point.y)) * m_graphics->GetInverseMatrix();
	SelectCursorOnline(pt);
}
LRESULT CAlgorithmView::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (m_animation.load(std::memory_order_relaxed)) return 0;

	CPoint point(lParam);

	UINT elflags;
	D2D1::MyPoint2F pt = D2D1::Point2F((float)point.x, (float)point.y) * m_graphics->GetInverseMatrix();
	auto founded = qtRoot.FindElement(pt.x, pt.y, elflags);
	if (founded != nullptr) {
		bool showed;
		if (!alg_->IsOnline()) {
			showed = founded->ShowPropertiesDialog(editor_, alg_);
		} else {
			showed = founded->ShowOnlineDialog(alg_.get());
		}
		if (showed) {
			Invalidate();
			SetFocus();
		}
	}
	return 0;
}

std::optional<HBRUSH> CAlgorithmView::CustomActiveColor() const noexcept {
	return alg_->IsOnline() ? gdiGlobal.brOnlineTab : std::optional<HBRUSH>{};
}

LRESULT CAlgorithmView::OnStartOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (alg_->IsUndoRedo()) {
		int res = ::MessageBoxA(m_hWnd, "You must save the changes in order to launch online mode. Do you want to save it now?", "", MB_ICONINFORMATION | MB_YESNO);
		if (res == IDYES) {
			Save();
			alg_->StartOnline();
		}
	} else {
		UnSelectSelected();
		m_prevSelectedOnline = nullptr;
		alg_->StartOnline();
	}
	m_tabview.Invalidate();
	return 0;
}
LRESULT CAlgorithmView::OnStopOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	alg_->StopOnline();
	if (m_prevSelectedOnline != nullptr) {
		m_prevSelectedOnline->SetSelect(FALSE);
		m_prevSelectedOnline = nullptr;
	}
	m_tabview.Invalidate();
	Invalidate();
	return 0;
}
LRESULT CAlgorithmView::OnUploadToDevice(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//	g_outputWndPtr->ClearOutput();
	//	m_animation = true;
	//	m_animation_thread = true;
	//	std::thread t(&CAlgorithmView::AnimationLoop, this);
	RenderBlur();
	alg_->UploadToDevice(true);
	Invalidate();
	//	m_animation_thread = false;
	//	t.join();
	//	Invalidate();
	return 0;
}
LRESULT CAlgorithmView::OnAlgorithmProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//	alg_->ShowProperties();
	SetFocus();
	return 0;
}
LRESULT CAlgorithmView::OnBlockProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);
	bool showed = m_selElements[0]->ShowPropertiesDialog(editor_, alg_);
	if (showed) {
		Invalidate();
		SetFocus();
	}
	return 0;
}

LRESULT CAlgorithmView::OnBlockExpandInputs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);

#ifdef DEBUG
	auto dyn_block = dynamic_cast<CDynamicBlock*>(m_selElements[0]);
	ATLASSERT(dyn_block != nullptr);
#else
	auto dyn_block = static_cast<CDynamicBlock*>(m_selElements[0]);
#endif // DEBUG

	std::string title = "[" + dyn_block->GetName() + "]: Inputs number";

	size_t value = dyn_block->top->slots.i_size();

	if (mctrl::show_dlg_simple_input(
		"Enter new inputs' count:",
		mctrl::make_numeric_edit(nullptr, value, [](size_t value) { if (value < 2 || value > 32) throw input_error("Set the number of inputs between 2 and 32"); })
	) != IDOK || value == dyn_block->top->slots.i_size()) return 0;

	RemoveFromQuadrant(dyn_block, &qtRoot);
	dyn_block->ExpandReduceInputs(value, *alg_->GetBlocks()->GetBlockFactory());
	PasteToQuadrant(dyn_block, &qtRoot);
	
	alg_->set_modified();
	Invalidate();

	return 0;
}

LRESULT CAlgorithmView::OnBlockConfigurableBlockEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);
	auto block = static_cast<CConfigurableBlock*>(m_selElements[0]);

	CDlg_ConfigurableBlock dlg(block);
	if (dlg.DoModal() == IDCANCEL) return 0;
	if (dlg.removed.empty() && dlg.added.empty()) return 0;

	for (auto& slot : dlg.removed) {
		auto var = slot->e_slot->GetVariableAsNode();
		if (var && (*var)->IsGood()) {
			alg_->actions.AddAction<DatabaseAction_ConfigurableSlotRemoved>(slot);
		}
		block->DisableSlot(slot);
		qtRoot.DeleteElement(slot->e_slot.get());
		qtRoot.DeleteElement(slot->e_slot->GetTextElement());
	}

	for (auto& slot : dlg.added) {
		auto var = slot->e_slot->GetVariableAsNode();
		if (var) {
			alg_->actions.RemoveAction(slot.id(), DATABASE_ACTION_CONFIGURABLE_SLOT_REMOVED);
		}
		block->EnableSlot(slot);
		qtRoot.AddElement(slot->e_slot.get());
		qtRoot.AddElement(slot->e_slot->GetTextElement());
	}

	alg_->set_modified();

	Invalidate();

	return 0;
}

LRESULT CAlgorithmView::OnBlockFucntionEquation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);
	auto block = static_cast<CBlockFunction*>(m_selElements[0]);

	CDlg_BlockFunctionEquation dlg(block);
	if (dlg.DoModal() == IDCANCEL) return 0;

	alg_->set_modified();

	return 0;
}

LRESULT CAlgorithmView::OnRemoveSliderThing(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);
	auto element = static_cast<CSliderThing*>(m_selElements[0]);

	auto cmd = new DeleteElementFromGroupSimple(element, element->GetParent(), &qtRoot);
	editor_->AddCommand(cmd, true);
	
	return 0;
}

LRESULT CAlgorithmView::OnAddTimePoint(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ATLASSUME(m_selElements.size() == 1);
	auto slider = static_cast<CScheduleSlider*>(m_selElements[0]);

	auto the_thing = editor_->GetBlockFactory()->CreateSliderThing(slider);

	auto cmd = new InsertElementToGroupSimple(the_thing, slider, &qtRoot);
	editor_->AddCommand(cmd, true);
	
	return 0;
}

LRESULT CAlgorithmView::OnCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (alg_->IsOnline()) return 0;
	OnCopy(wNotifyCode, wID, hWndCtl, bHandled);
	OnDelete(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

LRESULT CAlgorithmView::OnUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (alg_->IsOnline()) return 0;
	editor_->Undo();
	Invalidate();
	return 0;
}

LRESULT CAlgorithmView::OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (alg_->IsOnline()) return 0;
	editor_->Redo();
	Invalidate();
	return 0;
}

LRESULT CAlgorithmView::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (alg_->IsOnline()) return 0;
	m_curState = A_NOT_SELECTED;
	editor_->DeleteSelectedElements(&qtRoot);
	FillPropertyGrid(nullptr);
	m_nRectSelected = 0;
	Invalidate();
	return 0;
}

LRESULT CAlgorithmView::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (IsModified()) Save();
	return 0;
}

LRESULT CAlgorithmView::OnCopyAsLink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ASSERT(m_selElements.size() == 1);

	auto elems = std::make_unique<CBlockCompositionWrapper>(alg_);

	npsys::fbd_block_n block;
	auto parameter = static_cast<CParameter*>(m_selElements[0]);
	if (parameter->GetDirection() == CParameter::OUTPUT_PARAMETER) {
		block = alg_->GetBlocks()->GetBlockFactory()->CreateBlock(ID_CREATE_BLOCK_INPUT);
	} else {
		block = alg_->GetBlocks()->GetBlockFactory()->CreateBlock(ID_CREATE_BLOCK_OUTPUT);
	}

	auto input = static_cast<CParameter*>(block->e_block.get());
	input->SetParamType(PARAMETER_TYPE::P_INTERNAL_REF);
	input->GetSlot()->SetSlotType(new CInternalRef(parameter->GetSlot()));
	input->GetSlot()->CommitTypeChanges();

	elems->Push(block->e_block.get());

	omembuf omf;
	{
		boost::archive::binary_oarchive_special oa(omf);
		oa << elems;
	}
	if (!OpenClipboard()) return 0;

	::EmptyClipboard();

	auto len = omf.length();
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, len);
	ASSERT(hMem);

	void* ptr = ::GlobalLock(hMem);
	memcpy(ptr, omf.c_str(), len);
	::GlobalUnlock(hMem);

	::SetClipboardData(g_clf_blocks, hMem);
	::CloseClipboard();
	return 0;
}

LRESULT CAlgorithmView::OnCopySlotPathToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ASSERT(m_selElements.size() == 1);
	auto parameter = static_cast<CParameter*>(m_selElements[0]);
	auto path = wide(parameter->GetSlot()->top->path());
	
	if (!OpenClipboard()) return 0;
	::EmptyClipboard();

	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, path.length() * 2 + 2);
	ASSERT(hMem);

	void* ptr = ::GlobalLock(hMem);
	memcpy(ptr, path.c_str(), path.length() * 2);
	static_cast<wchar_t*>(ptr)[path.length()] = L'\0';
	::GlobalUnlock(hMem);

	::SetClipboardData(CF_UNICODETEXT, hMem);
	::CloseClipboard();

	return 0;
}

LRESULT CAlgorithmView::OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (m_selElements.size() == 0) return 0;

	auto elems = std::make_unique<CBlockCompositionWrapper>(alg_);
	for (auto elem : m_selElements) {
		if (elem->IsSelected()) {
			elems->Push(elem);
		}
	}

	m_selElements.clear();

	if (elems->Empty()) return 0;

	omembuf omf;
	{
		boost::archive::binary_oarchive_special oa(omf);
		oa << elems;
	}

	if (!OpenClipboard()) return 0;

	::EmptyClipboard();

	auto len = omf.length();
	auto hMem = ::GlobalAlloc(GMEM_MOVEABLE, len);
	ASSERT(hMem);

	void* ptr = ::GlobalLock(hMem);
	memcpy(ptr, omf.c_str(), len);

	::GlobalUnlock(hMem);
	::SetClipboardData(g_clf_blocks, hMem);
	::CloseClipboard();

	return 0;
}

LRESULT CAlgorithmView::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (!::IsClipboardFormatAvailable(g_clf_blocks)) return 0;
	if (!OpenClipboard()) return 0;

	auto hMem = GetClipboardData(g_clf_blocks);
	if (hMem != NULL) {
		SIZE_T size = ::GlobalSize(hMem);
		void* ptr = ::GlobalLock(hMem);

		boost::iostreams::array_source sink((char*)ptr, size);
		boost::iostreams::stream<boost::iostreams::array_source> stream(sink);
		boost::archive::binary_iarchive_special ia(stream);

		std::unique_ptr<CBlockCompositionWrapper> elems;
		ia >> elems;

		elems->Adapt(editor_);

		::GlobalUnlock(hMem);
		CloseClipboard();

		// setting up new unique names
		{
			CBlockCompositionBase::Container tmp;
			std::copy(editor_->begin(), editor_->end(), std::back_inserter(tmp));
			for (auto block : elems->blocks_) {
				const std::string pattern = block->e_block->GetTypeNameStr();
				block->e_block->SetName(
					CBlockCompositionBase::CreateUniqueName(
						tmp.cbegin(), tmp.cend(), pattern));
				tmp.push_back(block->e_block.get());
			}
		}

		editor_->SetSelect(false);

		m_selElements.clear();

		for (auto el : *elems.get()) {
			el->SetSelect(true);
			m_selElements.push_back(el);
		}

		auto& center = m_graphics->GetCenterOfView();

		elems->Move(CalcNearestPos(center.x), CalcNearestPos(center.y));

		elems->UpdateAllBezierLines();
		elems->ClearVariables();

		Insert(std::move(elems));

		if (m_selElements.size() > 1) {
			m_pMpr = &m_groupManipulator;
			m_curState = S_CURSOR::A_SELECTED;
		}
	} else {
		CloseClipboard();
	}

	return 0;
}

bool CAlgorithmView::IsModified() {
	return alg_->IsUndoRedo() || alg_->is_modified();
}

#include <npdb/debug.h>

void print_debug_blocks(odb::node_list<npsys::fbd_block_n>& blocks) {
	for (auto& block : blocks) {
		std::cout << block->get_name() << " loaded at: " << std::hex << block->e_block.get() << std::dec << std::endl;
		for (auto& slot : block->slots) {
			std::cout << '\t' << slot->get_name();
			std::cout << " loaded at: " << std::hex << slot->e_slot.get() << std::dec;
			std::cout << " slt loaded at : " << std::hex << slot->e_slot->GetSlotType() << std::dec << std::endl;
		}
	}
}

void print_editor_blocks(CElement* root) {
	Iter::PreorderIterator<CElement*> it(root);
	for (; !it.IsDone(); it.Next()) {
		if (auto slot = dynamic_cast<CSlot*>((*it))) {
			std::cout << "slot: loaded at: " << std::hex << slot <<
				" slot: loaded at: " << slot->GetSlotType() << std::dec << std::endl;
		}
	}
}

void CAlgorithmView::Save() {
	//	std::cout << "b" << std::endl;
	//	std::cout << alg_->fbd_blocks << std::endl;
	//	print_debug_blocks(alg_->fbd_blocks);
	//	print_editor_blocks(alg_->GetBlocks());
	odb::Batch batch;
	
	const auto& mat = m_graphics->GetMatrix();
	alg_->view_position->SavePosition(mat._31, mat._32, mat._11);
	if (alg_->view_position->is_modified()) alg_->view_position.store();
	
	alg_->Save();
	
	batch.exec();
	//	print_debug_blocks(alg_->fbd_blocks);
	//	print_editor_blocks(alg_->GetBlocks());
	//	std::cout << "e" << std::endl;
	//	std::cout << alg_->fbd_blocks << std::endl;
	//	alg_->ShowBlocks();
}

BOOL CAlgorithmView::CanUndo() {
	return editor_->IsUndoCommands();
}

BOOL CAlgorithmView::CanRedo() {
	return editor_->IsRedoCommands();
}

BOOL CAlgorithmView::CanStartOnline() {
	return alg_->GetStatus() > npsys::CAlgorithm::status_assigned && !alg_->IsOnline();
}
BOOL CAlgorithmView::CanStopOnline() {
	return alg_->IsOnline();
}
BOOL CAlgorithmView::CanUpload() {
	return alg_->GetStatus() > npsys::CAlgorithm::status_not_assigned;
}

/*
void CAlgorithmView::OnSetFocus(CWnd* pOldWnd) {
	CView::OnSetFocus(pOldWnd);
	m_pWndPropList->SetOwner(this);
}
*/

LRESULT CAlgorithmView::OnUpdateOnlineData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	alg_->UpdateData(reinterpret_cast<std::vector<nps::server_value>*>(wParam));
	return 0;
}

LRESULT CAlgorithmView::OnOnlineConnectionLoss(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	alg_->UpdateData(nullptr);
	return 0;
}

LRESULT CAlgorithmView::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (m_animation) return 0;

	CPoint point(lParam);
	UINT fwKeys = GET_KEYSTATE_WPARAM(wParam);
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

	const D2D1::Matrix3x2F& invMat = m_graphics->GetInverseMatrix();;
	D2D1_POINT_2F ptSc, pt, ptSc2;

	ScreenToClient(&point);

	ptSc.x = static_cast<float>(point.x);
	ptSc.y = static_cast<float>(point.y);

	pt = ptSc * invMat;

	auto scale = m_graphics->GetMatrix()._11;

	if (zDelta > 0) {
		scale += 0.25f;
	} else {
		scale -= 0.25f;
	}

	if (scale >= 6.f)
		scale = 6.f;

	if (scale <= 0.2f) {
		scale = 0.2f;
		m_graphics->EnableGrid(false);
	} else {
		m_graphics->EnableGrid(true);
	}

	ptSc2.x = pt.x * scale;
	ptSc2.y = pt.y * scale;

	m_graphics->SetMatrix(D2D1::Matrix3x2F(scale, 0, 0, scale, -(ptSc2.x - ptSc.x), -(ptSc2.y - ptSc.y)));

	Invalidate();

	return 0;
}

LRESULT CAlgorithmView::OnMButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (m_animation) return 0;
	SetCapture();
	m_ptMove = CPoint(lParam);
	return 0;
}
LRESULT CAlgorithmView::OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (m_animation) return 0;
	ReleaseCapture();
	return 0;
}

/*
CAlgorithmExt* CAlgorithmView::GetOpenedAlg(int alg_id) {
	for (auto& a : g_OpenedAlg) {
		if (a->GetId() == alg_id)
			return a;
	}
	return nullptr;
}
*/

LRESULT CAlgorithmView::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (m_animation) return 0;
	if (alg_->IsOnline()) return 0;

	CPoint point(lParam);
	m_ptMenu = point;

	UINT flags = static_cast<UINT>(wParam), elflags;
	D2D1::MyPoint2F pt = D2D1::Point2F((float)point.x, (float)point.y) * m_graphics->GetInverseMatrix();
	auto elem = qtRoot.FindElement(pt.x, pt.y, elflags);

	ClientToScreen(&point);

	if (elem == nullptr) {
		bool invalidate = !m_selElements.empty();

		UnSelectSelected();

		if (invalidate) Invalidate();

		UINT isClipboard = ::IsClipboardFormatAvailable(g_clf_blocks) ? MF_ENABLED : MF_DISABLED;
		CMenuHandle menu = m_menu.GetSubMenu(0);

		menu.EnableMenuItem(ID_EDIT_PASTE, isClipboard);
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, m_hWnd);
	} else {
		if (m_selElements.size() > 1) {
			m_ChangeCursor = AlphaCursor::Arrow;
			CMenuHandle menu = m_menu_selected.GetSubMenu(0);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, m_hWnd);
		} else if (m_selElements.size() == 1 && m_selElements[0] == elem) {
			m_ChangeCursor = AlphaCursor::Arrow;
			auto menu = CMenu(elem->GetContextMenu().m_hMenu);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, m_hWnd);
		} else {
			UnSelectSelected();

			elem->SetSelect(TRUE);
			m_selElements.push_back(elem);

			Invalidate();
			auto menu = CMenu(elem->GetContextMenu().m_hMenu);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, m_hWnd);
		}

	}
	return 0;
}

void CAlgorithmView::CreateBlock(WORD wID, const CPoint* pt) {
	//	odb::Batch batch;
	Insert(
		alg_->GetBlocks()->GetBlockFactory()->CreateBlock(wID),
		pt);
	//	batch.exec();
}

LRESULT CAlgorithmView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	switch (wParam)
	{
	case VK_ESCAPE:
		g_ribbonState.ResetBlockGallery();
		break;
	default:
		break;
	}
	//	}
	return 0;
}

LRESULT CAlgorithmView::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//	if (vw_ && vw_->GetFocus()) {
	//		vw_->OnChar((char)wParam);
	//		Invalidate();
	//	}
	return 0;
}

LRESULT CAlgorithmView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	TimerId id = (TimerId)(wParam);

	switch (id) {
	case TMR_DEBUG_INFO:
		KillTimer(TMR_DEBUG_INFO);
		if (tooltip_was_showed_) return 0;
		tooltip_was_showed_ = true;

		if (pointed_element_) {
#ifdef DEBUG
			D2D1_RECT_F rc;
			rc.left = pt_cursor_.x + 10;
			rc.top = pt_cursor_.y - 20;
			rc.right = rc.left + 600;
			rc.bottom = rc.top + 16;

			std::stringstream ss;
			pointed_element_->PrintDebugTooltip(ss);
			std::string line;
			if (ss.rdbuf()->in_avail()) {
				m_graphics->BeginDraw();
				while (std::getline(ss, line)) {
					m_graphics->FillRect(rc, SolidColor::LightGray);
					auto line_w = wide(line);
					m_graphics->DrawTextLeft(line_w.c_str(), static_cast<int>(line_w.length()), rc, SolidColor::Green);
					rc.top += 16;
					rc.bottom += 16;
				}
				m_graphics->EndDraw();
			}
#endif
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	};

	return 0;
}

LRESULT CAlgorithmView::OnPropertyChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	using CMyVariant = npsys::CMyVariant;
	const auto& variant = *reinterpret_cast<const CPropertyGrid::Variant*>(wParam);
	switch (lParam) {
	case PRB::ID_PR_ALG_SCAN_PERIOD:
	{
		const auto& v = boost::get<CMyVariant>(variant);
		alg_->SetScanPeriodMs(v.as<uint16_t>());
		alg_->set_modified();
		break;
	}
	case PRB::ID_PR_NAME:
	{
		if (m_selElements.size() != 1) break;
		auto elem = m_selElements[0];
		const auto& cv = boost::get<CMyVariant>(variant);
		const auto& name = cv.as<std::string>();
		if (!editor_->CheckName(cv.as<std::string>(), elem)) {
			auto& v = const_cast<CMyVariant&>(cv);
			v = elem->GetName();
			::MessageBoxA(m_hWnd, "The name is already exists", "", MB_ICONEXCLAMATION);
			break;
		}
		m_selElements[0]->SetName(name);
		alg_->set_modified();
		Invalidate();
		break;
	}
	default:
		if (m_selElements.size() != 1) break;
		m_selElements[0]->OnPropertyChanged(wParam, lParam);
		alg_->set_modified();
		Invalidate();
		break;
	}
	return 0;
}

// CAlgorithmMultiManipulator
S_CURSOR CAlgorithmView::CAlgorithmMultiManipulator::MouseButtonDown(CPoint point, CAlgorithmView* pWnd) {
	CBlockComposition* pBlocks = pWnd->alg_->GetBlocks();
	m_selBlocks.clear();

	Traversal<CBlockVisitor>(pBlocks, m_selBlocks);

	std::vector<CLine*> lines;

	for (auto& l : m_selBlocks) l->CollectLines(lines);

	m_lines.assign(lines.begin(), lines.end());

	ASSERT(m_selBlocks.empty() == false);

	const D2D1::Matrix3x2F& invM = pWnd->GetGraphics()->GetInverseMatrix();
	const D2D1::MyPoint2F& pos = static_cast<D2D1::MyPoint2F&>(const_cast<D2D1_POINT_2F&>(m_selBlocks[0]->GetPosition()));

	_prevpos = _oldpos = pos;

	D2D1::MyPoint2F m_ptCursor = D2D1::Point2F((float)point.x, (float)point.y) * invM;
	_offset = m_ptCursor - _oldpos;

	drag = 0;
	return S_CURSOR::A_GROUP;
}

void CAlgorithmView::CAlgorithmMultiManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	const D2D1::Matrix3x2F& invM = pWnd->GetGraphics()->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pWnd->GetGraphics()->GetMatrix();
	const float& scale_f = matrix._11;

	D2D1::MyPoint2F pt;

	pt.x = (float)point.x - _offset.x * scale_f;
	pt.y = (float)point.y - _offset.y * scale_f;

	if (!drag) {
		pWnd->GetGraphics()->SetEditingMode(true);
		drag = 1;
		pWnd->SetCursor(AlphaCursor::Drag);
//  ::SetCursor(LoadCursor(IDC_SIZEALL));

		for (auto& el : m_selBlocks) {
			el->BeginEditState();
			RemoveFromQuadrant(el, &pWnd->qtRoot);
		}
		
		for (auto& l : m_lines) {
			pWnd->qtRoot.DeleteElement(l);
		}
	} else {
		pt = pt * invM;

		float dx = pWnd->CalcNearestPos(pt.x - _prevpos.x);
		float dy = pWnd->CalcNearestPos(pt.y - _prevpos.y);

		for (auto el : m_selBlocks) el->MoveDxDy((float)dx, (float)dy);
		for (auto& l : m_lines) l->Create();

		_prevpos = static_cast<D2D1::MyPoint2F&>(const_cast<D2D1_POINT_2F&>(m_selBlocks[0]->GetPosition()));

		pWnd->Invalidate();
	}
}
void CAlgorithmView::CAlgorithmMultiManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	pWnd->SetCursor(AlphaCursor::Arrow);
	if (!drag) return;

	const D2D1::MyPoint2F& pos = static_cast<D2D1::MyPoint2F&>(const_cast<D2D1_POINT_2F&>(m_selBlocks[0]->GetPosition()));
	float dx = pos.x - _oldpos.x;
	float dy = pos.y - _oldpos.y;

	for (auto& l : m_lines) pWnd->qtRoot.AddElement(l);

	for (auto el : m_selBlocks) {
		el->EndEditState();
		PasteToQuadrant(el, &pWnd->qtRoot);
	}

	if (fabs(dx) > 0.001f || fabs(dy) > 0.001f) {
		auto cmd = new GroupCommand2<Command>;
		cmd->SetFinishCommand(new UpdateLinesCommand(m_lines, &pWnd->qtRoot));
		auto delta = D2D1::Point2F(dx, dy);
		for (auto el : m_selBlocks) {
			cmd->AddCommand(el->CreateMoveCommand(delta, &pWnd->qtRoot), false);
			el->EndEditState();
		}
		pWnd->alg_->GetBlocks()->AddCommand(cmd, false);
	}
	pWnd->GetGraphics()->SetEditingMode(false);
	pWnd->Invalidate();
}

void CAlgorithmView::CAlgorithmMultiManipulator::Draw(CGraphics* pGraphics) {
	for (auto& bl : m_selBlocks) bl->Draw(pGraphics);
	for (auto& l : m_lines) l->Draw(pGraphics);
}