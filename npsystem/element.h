// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/header.h>
#include "stuff.h"
#include "quadrant.h"
#include "global.h"
#include "manipulator.h"
#include "blockpropertyid.h"
#include <bitset>

#define DECLARE_TYPE_NAME(type_name) \
	virtual std::string_view GetTypeName() const noexcept { return type_name; } 

class CGroup;
class Command;
class MoveCommand;
class CGraphics;
class CBlockComposition;
class CBlockCompositionWrapper;
class CPropertyGrid;

namespace npsys {
class CAlgorithmExt;
}


#define EL_INVALIDATE 1
#define VALUE_CHANGED 2

class CElement {
public:
	friend QuadrantAdapter<CElement, float>;
	typedef QuadrantNode<CElement, float, 60> MyQuadrantNode;
	typedef Iter::Iterator<CElement*> _Iterator;
private:
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & z_order_;
		ar & visible_;
		ar & rect_.left;
		ar & rect_.top;
		ar & rect_.right;
		ar & rect_.bottom;

		if constexpr (Archive::is_loading::value) {
			if constexpr (is_special_archive_v<Archive> == false) {
				flags_.set(created_new, false);
			}
		} else {
			flags_.set(created_new, false);
		}
	}

	static const std::string emptyString_;
	// Serializable
	int	z_order_;
	bool visible_;
	// Not Serializable
	std::bitset<8> flags_;
	enum { hovered = 0, selected, edit_mode, hit_access, created_new, geometry_realization_dirty };
	int render_frame_;
protected:
	D2D1_RECT_F rect_;

	auto IsGeometryRealizationDirty() { return !flags_.test(geometry_realization_dirty); }
	void SetGeometryRealizationFlag(bool dirty) { flags_.set(geometry_realization_dirty, !dirty); }
public:
	virtual std::string_view GetTypeName() const noexcept = 0;
	virtual void Draw(CGraphics* graphics) = 0;
	virtual void PrintDebugTooltip(std::stringstream& ss) {}
	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags);
	virtual void Visit(CBlockVisitor& v) = 0;
	virtual CManipulator* GetManipulator() = 0;
	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept;
	virtual bool IsSerializable() const noexcept { return true; }
	virtual	_Iterator* CreateIterator();
	void GetRect(D2D1_RECT_F& rc) const noexcept { rc = rect_; }
	void SetRect(const D2D1_RECT_F& rc) noexcept {  rect_ = rc; }
	void SetRect(float x, float y, float width, float height) noexcept { 
		rect_ = D2D1::RectF(x, y, x + width, y + height); 
	}
	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);
	virtual bool ShowPropertiesDialog(CBlockComposition* blocks, npsys::algorithm_n& alg) { return false; }
	virtual bool ShowOnlineDialog(npsys::CAlgorithmExt* alg) { return false; }
	virtual Command* DeleteElement(CBlockComposition* pBlocks, MyQuadrantNode* qtRoot) { ASSERT(FALSE); return NULL; }
	virtual AlphaCursor GetCursor() { return AlphaCursor::Arrow; }
	virtual AlphaCursor GetOnlineCursor() { return AlphaCursor::Arrow; }
	virtual const std::string& GetName() const noexcept { return emptyString_; }
	virtual void Transform(const D2D1::Matrix3x2F& matrix) {}
	virtual void InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept;

	virtual CMenuHandle GetContextMenu() { 
		CMenu menu;
		menu.CreatePopupMenu();
		
		menu.AppendMenu(MF_STRING, ID_EDIT_CUT, L"Cut");
		menu.AppendMenu(MF_STRING, ID_EDIT_COPY, L"Copy");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_BLOCK_PROPERTIES, L"Properties");
	
		return menu.Detach();
	}

	virtual	MoveCommand* CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot);
	virtual bool IsNode() const noexcept { return false; }
	virtual void SetName(const std::string&) noexcept { ASSERT(FALSE); }

	auto IsHover() const noexcept { return flags_.test(hovered); }
	void SetHover(BOOL hover) noexcept { flags_.set(hovered, hover); }
	auto IsSelected() const noexcept { return flags_.test(selected); }
	void SetSelect(BOOL select) noexcept { flags_.set(selected, select); }
	auto IsEditState() const { return flags_.test(edit_mode); }
	void BeginEditState() { flags_.set(edit_mode, true);}
	void EndEditState() {  flags_.set(edit_mode, false); }
	auto IsHitAccess() { return flags_.test(hit_access); }
	void SetHitAccess() { flags_.set(hit_access, true); }
	void ResetHitAccess() { flags_.set(hit_access, false); }
	auto IsCreatedNew() { return flags_.test(created_new); }
	void SetZOrder(int z_order) { z_order_ = z_order; }
	int GetZOrder() { return z_order_; }
	bool IsVisible() const noexcept { return visible_; }
	void ShowElement(bool show) noexcept { visible_ = show; }

	const std::string GetTypeNameStr() const noexcept {
		auto str = GetTypeName(); 
		return std::string(str); 
	}
	float Width() const noexcept { return rect_.right - rect_.left; }
	const D2D1_RECT_F& GetBounds() { return rect_; }

	const D2D1_POINT_2F&	GetPosition() {
		return reinterpret_cast<D2D1_POINT_2F&>(rect_);
	}

	D2D1_POINT_2F GetMiddle() {
		return D2D1::Point2F(
			rect_.left + (rect_.right - rect_.left) / 2,
			rect_.top + (rect_.bottom - rect_.top) / 2);
	}

	D2D1_SIZE_F GetSize() {
		return D2D1::SizeF(
			rect_.right - rect_.left,
			rect_.bottom - rect_.top);
	}

	BOOL RectInRect(const D2D_RECT_F& rect) const {
		if ((rect_.left < rect.left) || (rect_.right > rect.right)) return FALSE;
		if ((rect_.top < rect.top) || (rect_.bottom > rect.bottom)) return FALSE;
		return TRUE;
	}

	CElement() {
		z_order_ = 1; // 0 for lines
		visible_ = true;
		rect_ = D2D1::RectF();
		render_frame_ = 0;
		flags_.set(created_new, true);
	}

	CElement(const CElement& el) {
		z_order_ = el.z_order_;
		visible_ = el.visible_;
		rect_ = el.rect_;
		flags_.set(hit_access, el.flags_[hit_access]);
		flags_.set(created_new, true);
		render_frame_ = 0;
	}

	virtual ~CElement() = default;
};


template<>
struct QuadrantAdapter<CElement, float> {
	typedef TQuadrantPoint<float> QuadrantPoint;
	typedef TQuadrantRect<float> QuadrantRect;

	inline static const QuadrantPoint& GetPosition(CElement* pElem) {
		return reinterpret_cast<QuadrantPoint&>(pElem->rect_);
	}
	inline static const QuadrantRect& GetBounds(CElement* pElem) {
		return reinterpret_cast<QuadrantRect&>(pElem->rect_);
	}
	inline static bool HitTest(CElement* pElem, float x, float y, UINT& flags) {
		return pElem->HitTest(D2D1::Point2F(x, y), flags);
	}
	inline static const QuadrantRect& GetVisibleArea(const D2D1_RECT_F& rect) {
		return reinterpret_cast<QuadrantRect&>(const_cast<D2D1_RECT_F&>(rect));
	}
	inline static const int IsRendered(CElement* pElem, int render_frame) {
		return pElem->render_frame_ == render_frame;
	}
	inline static const void SetRenderedState(CElement* pElem, int render_frame) {
		pElem->render_frame_ = render_frame;
	}
	inline static const int GetZOrder(CElement* pElem) {
		return pElem->z_order_;
	}
	inline static const int IsHitAccess(CElement* pElem) {
		return pElem->IsHitAccess();
	}
	inline static void Draw(CElement* elem, CGraphics* graphics) {
		return elem->Draw(graphics);
	}
};

class CElementManipulator : public CManipulator {
protected:
	D2D1::MyPoint2F _offset; // distance between pointer click and top-left corner of the element
	D2D1::MyPoint2F _oldpos;
	CElement* element_ = nullptr;
	CPoint _oldScreenCursor;
	bool was_draged_ = false;
public:
	virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);
	virtual void Drag(CPoint point, CAlgorithmView* pWnd);
	virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
	virtual void Draw(CGraphics* pGraphics);

	CElementManipulator(CElement* element) 
		: element_(element) 
	{
	}
};

class CSliderManipulator : public CElementManipulator {
	CGroup* slider_;
public:
	virtual void Drag(CPoint point, CAlgorithmView* pWnd);
	void AfterDeserialization(CGroup* slider) { slider_ = slider; }

	CSliderManipulator(CElement* element, CGroup* slider) 
		: CElementManipulator(element) 
		, slider_(slider)
	{
	}
};

class CGraphElement {
	int	tmp_id_;
public:
	void SetTmpId(int id) noexcept { tmp_id_ = id; }
	int GetTmpId() const noexcept { return tmp_id_; }
};


class CBlockComposition;
class CSlotType;
class CBlock;
class CSlot;
class CSliderTimeChart;
class CSliderThing;
class CScheduleSlider;
class CBlockSchedule;

class CBlockFactory {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & last_id_;
	}

	void CreateBlock(CBlock* block);
	CScheduleSlider* CreateScheduleSlider(CBlockSchedule* parent);
	CSliderTimeChart* CreateSliderTimeChart(CScheduleSlider* parent);
	npsys::fbd_slot_n CreateSlot(const std::string& name, CSlot* slot, npsys::fbd_block_n& block);
	int next_id() noexcept { return last_id_++; }
	
	int last_id_ = 0;
	CBlockComposition* blocks_;
public:
	npsys::fbd_block_n CreateBlock(WORD wID);
	
	int	SetNewId(class CSlot*);
	void CreateInput(npsys::fbd_block_n&);
	void CreateOutput(npsys::fbd_block_n&);
	void CreateOr(npsys::fbd_block_n&);
	void CreateAnd(npsys::fbd_block_n&);
	void CreateRsTrigger(npsys::fbd_block_n&);
	void CreateNot(npsys::fbd_block_n&);
	void CreatePositiveEdge(npsys::fbd_block_n&);
	void CreateNegativeEdge(npsys::fbd_block_n&);
	void CreateAnyEdge(npsys::fbd_block_n&);
	void CreateDelay(npsys::fbd_block_n&);
	void CreateBinaryEncoder(npsys::fbd_block_n&);
	void CreateBinaryDecoder(npsys::fbd_block_n&);
	void CreateAdd(npsys::fbd_block_n&);
	void CreateSub(npsys::fbd_block_n&);
	void CreateMul(npsys::fbd_block_n&);
	void CreateDiv(npsys::fbd_block_n&);
	void CreateComparator(npsys::fbd_block_n&);
	void CreateFunction(npsys::fbd_block_n&);
	void CreatePID(npsys::fbd_block_n&);
	void CreateAlarmHigh(npsys::fbd_block_n&);
	void CreateAlarmLow(npsys::fbd_block_n&);
	void CreateTime(npsys::fbd_block_n&);
	void CreateSchedule(npsys::fbd_block_n&);
	void CreateCounter(npsys::fbd_block_n&);
	npsys::fbd_slot_n CreateInputSlot(const std::string&, npsys::fbd_block_n&, CSlotType*);
	npsys::fbd_slot_n CreateOutputSlot(const std::string&, npsys::fbd_block_n&, CSlotType*);
	CSliderThing* CreateSliderThing(CScheduleSlider* parent);

	CBlockFactory(CBlockComposition* blocks = nullptr) 
		: blocks_(blocks) 
	{
	}
};