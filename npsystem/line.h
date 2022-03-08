// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "element.h"
#include "color.h"
#include "graphics.h"

class CInputSlot;
class COutputSlot;
class CBlockVisitor;

class CLineBase
	: public CElement
	, public CGraphElement {
public:
	enum class LineWidth {
		Normal, Bold
	};
protected:
	static constexpr auto c_arrow_h = 4.0f;
	
	LineWidth width_ = LineWidth::Normal;
	SolidColor color_ = SolidColor::Black;
	
	wrl::ComPtr<ID2D1PathGeometry1> m_geometry;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationStroked;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationStrokedBold;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationFilled;

	enum ATDIR { SN = 1 << 31, NS = 0, EW = 1 << 31, WE = 0 };
	static void MakeArrowTipHorizontal(ATDIR dir, const D2D1::MyPoint2F& p1, ID2D1GeometrySink* sink);
	static void MakeArrowTipVertical(ATDIR dir, const D2D1::MyPoint2F& p1, ID2D1GeometrySink* sink);
public:
	void SetWidthAndColor(LineWidth width, SolidColor color) noexcept {
		width_ = width;
		color_ = color;
	}

	void PreDraw(CGraphics* graphics);
	void Error(int error);
	
	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags);
	virtual void Draw(CGraphics* pGraphics);

	CLineBase() {
		SetZOrder(0);
		SetHitAccess();
	}
};

class CBlockCompositionBase;

class CBlockLine 
	: public CLineBase {
public:
	virtual void ConnectSlots(bool recreate_dependencies, CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot = nullptr) = 0;
	virtual void Disconnect(CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot) = 0;
	virtual std::unique_ptr<Command> DeleteElement(CBlockComposition* pBlocks, MyQuadrantNode* qtRoot);
};

template<typename TSlot, typename TSlotIn, typename TSlotOut, typename MostDerrived>
class CLineT : public CBlockLine {
	MostDerrived& derived() { return static_cast<MostDerrived&>(*this); }

	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
		ar & slot_in_->GetId();
		ar & slot_out_->GetId();
	}
	
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		ar & slot_in_id_;
		ar & slot_out_id_;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		_serialize_(ar, file_version);
	}

	D2D1::MyPoint2F	pt1_;
	D2D1::MyPoint2F	pt2_;
protected:
	template<class Archive>
	void _serialize_(Archive& ar, const unsigned int file_version) { // to support old CLine hierarchy
		ar& boost::serialization::base_object<CElement>(derived());
		boost::serialization::split_member(ar, derived(), file_version);
	}

	uint32_t slot_in_id_;
	uint32_t slot_out_id_;

	TSlotIn* slot_in_;
	TSlotOut* slot_out_;

	void CalculateBounds() {
		if (pt1_.x < pt2_.x) {
			rect_.left = pt1_.x;
			rect_.right = pt2_.x;
		} else {
			rect_.left = pt2_.x;
			rect_.right = pt1_.x;
		}
		if (pt1_.y > pt2_.y) {
			rect_.top = pt2_.y;
			rect_.bottom = pt1_.y;
		} else {
			rect_.top = pt1_.y;
			rect_.bottom = pt2_.y;
		}
	}
public:
	uint32_t GetInID() { return slot_in_id_; }
	uint32_t GetOutID() { return slot_out_id_; }
	
	const D2D1_POINT_2F& GetP1() { return pt1_; }
	const D2D1_POINT_2F& GetP2() { return pt2_; }

	void Update() {
		ASSERT(slot_in_);
		ASSERT(slot_out_);

		slot_in_->UpdateLinePosition();
		slot_out_->UpdateLinePosition();
	}

	void SetP1(D2D1_POINT_2F pt) noexcept {
		if (pt != pt1_) SetGeometryFlag(true);
		pt1_ = pt;
	}

	void SetP2(D2D1_POINT_2F pt) noexcept {
		if (pt != pt2_) SetGeometryFlag(true);
		pt2_ = pt;
	}

	TSlotIn* GetIn() { return slot_in_; }
	TSlotOut* GetOut() { return slot_out_; }

	TSlot* GetAnotherSlot(const TSlot* slot, int* dir = nullptr) {
		if (slot == slot_in_) {
			if (dir) (*dir) = 1;
			return slot_out_;
		} else if (slot == slot_out_) {
			if (dir) (*dir) = -1;
			return slot_in_;
		} else {
			if (dir) (*dir) = 0;
			return nullptr;
		}
	}

	template<typename TConnectable>
	bool SetSlot(TConnectable* connectable, bool checking) {
		ATLASSERT(connectable);
		return connectable->ConnectToLine(&derived(), checking);
	}

	bool SetSlot(TSlotIn* in) {
		if (slot_in_) return false;
		slot_in_ = in;
		return true;
	}

	bool SetSlot(TSlotOut* out) {
		if (slot_out_) return false;
		slot_out_ = out;
		return true;
	}

	CLineT(const CLineT&) = delete;
	CLineT& operator=(const CLineT&) = delete;

	CLineT() 
		: slot_in_(nullptr)
		, slot_out_(nullptr)
	{
	}
};

class CBlockComposition;

class Command_DeleteLine : public Command {
	CBlockLine* line_;
	CBlockComposition* editor_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	virtual void Execute();
	virtual void UnExecute();

	virtual ~Command_DeleteLine();
	Command_DeleteLine(CBlockLine* line, CBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot);
};

class Command_InsertLine 
	: public Command_DeleteLine {
public:
	virtual void Execute() { Command_DeleteLine::UnExecute(); }
	virtual void UnExecute() { Command_DeleteLine::Execute(); }
	Command_InsertLine(CBlockLine* line, CBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot)
		: Command_DeleteLine(line, blocks, qtRoot) {}
};