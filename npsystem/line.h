// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "element.h"
#include "color.h"

class CInputSlot;
class COutputSlot;
class CBlockVisitor;

class CLine
	: public CElement
	, public CGraphElement
{
public:
	enum class LineWidth {
		Normal, Bold
	};
private:
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int /*file_version*/) const {
		ar & slot_in_->GetId();
		ar & slot_out_->GetId();
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int /*file_version*/) {
		ar & slot_in_id_;
		ar & slot_out_id_;
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CElement>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}

	int _lenght;
	LineWidth m_lineWidth;
	D2D1::MyPoint2F	_p1;
	D2D1::MyPoint2F	_p2;
	int _bend;
	float dx_ = 0.0f;
	D2D1_POINT_2F _bend_point;
	CInputSlot* slot_in_;
	COutputSlot* slot_out_;
	uint32_t slot_in_id_;
	uint32_t slot_out_id_;
	float _offset_x;
	float _offset_y;
	SolidColor _colorOnline;

	wrl::ComPtr<ID2D1PathGeometry1> m_geometry;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationStroked;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationStrokedBold;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationFilled;

	void CreateBezier(bool predraw = false);
	void Error(int error);

	class CLineManipulator : public CManipulator {
	public:
		CLineManipulator(CLine* line) : _line(line) {}
		virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);
		virtual void Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
	private:
		CLine * _line = nullptr;
	} manipulator_;
public:
	DECLARE_TYPE_NAME("LINE")
	DECLARE_VISITOR()

	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags);
	virtual void Draw(CGraphics* pGraphics);
	virtual Command* DeleteElement(CBlockComposition* pBlocks, MyQuadrantNode* qtRoot);
	virtual	void Move(int x, int y) { }
	virtual	void MoveDxDy(int dx, int dy) {  }
	virtual CManipulator* GetManipulator() { return &manipulator_; }
	
	void PreDraw(CGraphics* pGraphics);
	const D2D1_POINT_2F& GetP1() { return _p1; }
	const D2D1_POINT_2F& GetP2() { return _p2; }
	void SetP1(D2D1_POINT_2F pt) {
		_p1 = pt;
		_p1.y += _offset_y;
	}
	void SetP2(D2D1_POINT_2F pt) { _p2 = pt; }
	void SetLinePosition(float offset_x, float offset_y) {
		_offset_x = offset_x + dx_;
		_offset_y = offset_y;
	}
	void SetWidthAndColor(LineWidth width, SolidColor color) {
		m_lineWidth = width;
		_colorOnline = color;
	}
	bool SetSlot(CSlot* slot);
	bool SetInputSlot(CInputSlot* in);
	bool SetOutputSlot(COutputSlot* out);
	uint32_t GetInID() { return slot_in_id_; }
	uint32_t GetOutID() { return slot_out_id_; }
	CInputSlot* GetIn() { return slot_in_; }
	COutputSlot* GetOut() { return slot_out_; }
	void Connect();
	void Disconnect();
	void Create();
	CSlot* GetAnotherSlot(CSlot* p_slot, /*OUT*/ int* dir = NULL);

	CLine();
	CLine(const CLine&) = delete;
	CLine& operator=(const CLine&) = delete;
};