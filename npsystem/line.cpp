// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "command.h"
#include "line.h"
#include "graphics.h"
#include <npsys/guid.h>

NP_BOOST_CLASS_EXPORT_GUID(CLine);
IMPLEMENT_VISITOR(CLine);

CLine::CLine() 
	: manipulator_(this)
	, slot_in_(nullptr)
	, slot_out_(nullptr)
	, m_lineWidth(LineWidth::Normal)
	, _colorOnline(SolidColor::Black) {
	SetZOrder(0);
	SetHitAccess();
	_offset_x = _offset_y = 0.0f;
}

void CLine::CreateBezier(bool predraw) {
	ID2D1GeometrySink* pSink = NULL;
	
	dis::Get().d2dFactory->CreatePathGeometry(m_geometry.ReleaseAndGetAddressOf());

	auto R = [] (float dist) {
		constexpr float max_r = 10.0f;
		if (dist < 2 * max_r) {
			return 0.5f * dist;
		} else {
			return max_r;
		}
	};

	D2D1::MyPoint2F& p1 = _p1;
	D2D1::MyPoint2F& p2 = _p2;

	// Write to the path geometry using the geometry sink.
	m_geometry->Open(&pSink);

	float dist = p2.y - p1.y;
	float dist_abs;

	if (((dist_abs = abs(dist)) < 5) && !predraw) {
		p2.y = p1.y;
		dist_abs = dist = 0;
	}

	if (p2.x < p1.x + 25.0f) {
		float r = R(dist_abs / 2.0f);
		float y_mid = p1.y + dist / 2.0f;
		pSink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLine(D2D1::Point2F(p1.x + 20.0f - r + _offset_x, p1.y));
		if (dist > 0) {
			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + 20.0f + _offset_x, p1.y + r),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(
				D2D1::Point2F(p1.x + 20.0f + _offset_x, y_mid - r)
			);
			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + 20.0f - r +_offset_x, y_mid),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(
				D2D1::Point2F(p2.x - 25.0f + r - _offset_x, y_mid)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x - 25.0f - _offset_x, y_mid + r),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);

			pSink->AddLine(
				D2D1::Point2F(p2.x - 25.0f - _offset_x, p2.y - r)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x - 25.0f + r - _offset_x, p2.y),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(p2);
		} else 
		{
			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + 20.0f + _offset_x, p1.y - r),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(
				D2D1::Point2F(p1.x + 20.0f + _offset_x, y_mid + r)
			);
			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + 20.0f - r + _offset_x, y_mid),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);

			pSink->AddLine(
				D2D1::Point2F(p2.x - 25.0f + r - _offset_x, y_mid)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x - 25.0f - _offset_x, y_mid - r),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);

			pSink->AddLine(
				D2D1::Point2F(p2.x - 25.0f - _offset_x, p2.y + r)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x - 25.0f + r - _offset_x, p2.y),
					D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(p2);
		}
	} else 
	{
		if (dist_abs > 0.001f) {
			float r = R(dist_abs);
			float sign = dist > 0 ? 1.0f : -1.0f;
			float x_mid = p1.x + (p2.x - p1.x) / 2;

		//	if (p1.y < p2.y) {
				x_mid -= _offset_x;
		//	} else {
			//	x_mid += _offset_x;
		//	}

			pSink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);
			pSink->AddLine(
				D2D1::Point2F(x_mid - r, p1.y)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(x_mid, p1.y + r * sign),
					D2D1::SizeF(r, r), 0, dist > 0 ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);

			pSink->AddLine(
				D2D1::Point2F(x_mid, p2.y - r * sign)
			);

			pSink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(x_mid + r, p2.y),
					D2D1::SizeF(r, r), 0, dist < 0 ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL)
			);
			pSink->AddLine(p2);
		} else 
		{
			pSink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);
			pSink->AddLine(
				D2D1::Point2F(p2.x - 6, p2.y)
			);
		}
	}
	// Arrow
	pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	pSink->BeginFigure(D2D1::Point2F(p2.x - 6, p2.y - 3), D2D1_FIGURE_BEGIN_FILLED);
	pSink->AddLine(
		D2D1::Point2F(p2.x, p2.y)
	);
	pSink->AddLine(
		D2D1::Point2F(p2.x - 6, p2.y + 3)
	);
	pSink->AddLine(
		D2D1::Point2F(p2.x - 6, p2.y - 3)
	);
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();
	pSink->Release();

	SetGeometryRealizationFlag(true);
}

void CLine::PreDraw(CGraphics* graphics) {
	CreateBezier(true);
	graphics->DrawGeometry(m_geometry.Get(), SolidColor::Black);
	graphics->FillGeometry(m_geometry.Get(), SolidColor::Black);
}

void CLine::Draw(CGraphics* graphics) {
	auto const normal_mode = !graphics->IsEditingMode();
	auto const geometry_dirty = IsGeometryRealizationDirty();

	if (normal_mode && geometry_dirty) {
		graphics->CreateStrokedGeometryRealization(
			m_geometry.Get(), 1.0f, m_geometryRealizationStroked.ReleaseAndGetAddressOf());
		graphics->CreateStrokedGeometryRealization(
			m_geometry.Get(), 1.6f, m_geometryRealizationStrokedBold.ReleaseAndGetAddressOf());
		graphics->CreateFilledGeometryRealization(m_geometry.Get(), m_geometryRealizationFilled.ReleaseAndGetAddressOf());
		SetGeometryRealizationFlag(false);
	}
	
	auto color = IsSelected() ? SolidColor::SelColor : _colorOnline;
	
	if (!geometry_dirty) [[likely]] {
		if (m_lineWidth == LineWidth::Normal) {
			graphics->DrawGeometryRealization(m_geometryRealizationStroked.Get(), color);
		} else {
			graphics->DrawGeometryRealization(m_geometryRealizationStrokedBold.Get(), color);
		}
		graphics->DrawGeometryRealization(m_geometryRealizationFilled.Get(), color);
	} else {
		graphics->DrawGeometry(m_geometry.Get(), color);
		graphics->FillGeometry(m_geometry.Get(), color);
	}
}

bool CLine::SetInputSlot(CInputSlot* in) {
	if (slot_in_) return false;
	slot_in_ = in;
	return true;
}

bool CLine::SetOutputSlot(COutputSlot* out) {
	if (slot_out_) return false;
	slot_out_ = out;
	return true;
}

bool CLine::SetSlot(CSlot* slot) {
	ATLASSERT(slot);
	return slot->ConnectToLine(this);
}

void CLine::Error(int error) {
	const char* msg = nullptr;
	
	switch (error) {
	case 1: 
		msg = "...";
		break;
	case 2:
		msg = "The input already has a connection";
		break;
	default:
		return;
	}
	if (msg) MessageBoxA(g_hMainWnd, msg, nullptr, MB_ICONWARNING);
}

void CLine::Connect() {
	ASSERT(slot_in_ && slot_out_);
	slot_in_->AddLine(this);
	slot_out_->AddLine(this);
	dx_ = static_cast<float>(std::max(slot_in_->GetSlotIndex(), slot_out_->GetSlotIndex())) * 2.5f;
}

void CLine::Disconnect() {
	ASSERT(slot_in_ && slot_out_);
	slot_in_->DisconnectLine(this);
	slot_out_->DisconnectLine(this);
}

void CLine::Create() {
	D2D1::MyPoint2F p1 = slot_out_->GetMiddle();
	D2D1::MyPoint2F p2 = slot_in_->GetMiddle();

	p1.x = slot_out_->GetParentBlock()->GetBounds().right;
	p2.x = slot_in_->GetParentBlock()->GetBounds().left - 1.5f;

	SetP1(p1);
	SetP2(p2);

	if (p1.x < p2.x) {
		rect_.left = p1.x;
		rect_.right = p2.x;
	} else {
		rect_.left = p2.x;
		rect_.right = p1.x;
	}
	if (p1.y > p2.y) {
		rect_.top = p2.y;
		rect_.bottom = p1.y;
	} else {
		rect_.top = p1.y;
		rect_.bottom = p2.y;
	}
	CreateBezier();
}

CSlot* CLine::GetAnotherSlot(CSlot* slot, int* dir) {
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

Command* CLine::DeleteElement(CBlockComposition* pBlocks, MyQuadrantNode* qtRoot) {
	return new DeleteLineCommand(this, pBlocks, qtRoot);
}

bool CLine::HitTest(const D2D1_POINT_2F& pt, UINT& flags) { 
	BOOL hit;
	m_geometry->StrokeContainsPoint(pt, 6, nullptr, nullptr, &hit);
	return static_cast<bool>(hit);
}

// CLineManipulator

S_CURSOR CLine::CLineManipulator::MouseButtonDown(CPoint point, CAlgorithmView* pWnd) {
	return S_CURSOR::A_ONE_ELEM;
}

void CLine::CLineManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {

}

void CLine::CLineManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
}
