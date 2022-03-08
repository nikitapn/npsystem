// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "line.h"
#include "blockcomposition.hpp"


void CLineBase::PreDraw(CGraphics* graphics) {
	graphics->DrawGeometry(m_geometry.Get(), SolidColor::Black);
	graphics->FillGeometry(m_geometry.Get(), SolidColor::Black);
}

void CLineBase::Draw(CGraphics* graphics) {
	auto const normal_mode = !graphics->IsEditingMode();

	if (normal_mode && IsGeometryRealizationDirty(graphics->GetId())) {
		graphics->CreateStrokedGeometryRealization(
			m_geometry.Get(), 1.0f, m_geometryRealizationStroked.ReleaseAndGetAddressOf());
		graphics->CreateStrokedGeometryRealization(
			m_geometry.Get(), 1.6f, m_geometryRealizationStrokedBold.ReleaseAndGetAddressOf());
		graphics->CreateFilledGeometryRealization(m_geometry.Get(), m_geometryRealizationFilled.ReleaseAndGetAddressOf());
		SetGeometryRealizationFlagAfterDraw(graphics->GetId());
	}
	
	auto const color = (IsSelected() ? SolidColor::SelColor : (IsHover() ? SolidColor::Green : color_));

	if (normal_mode) [[likely]] {
		if (width_ == LineWidth::Normal) {
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

void CLineBase::Error(int error) {
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

bool CLineBase::HitTest(const D2D1_POINT_2F& pt, UINT& flags) { 
	bool hit;
	{
		BOOL result;
		m_geometry->StrokeContainsPoint(pt, 6, nullptr, nullptr, &result);
		hit = static_cast<bool>(result);
	}

	if (IsHover() != hit) {
		SetHover(hit);
		flags |= EL_INVALIDATE;
	}

	return static_cast<bool>(hit);
}

void CLineBase::MakeArrowTipHorizontal(ATDIR dir, const D2D1::MyPoint2F& p1, ID2D1GeometrySink* sink) {
	float l = c_arrow_h;
	reinterpret_cast<uint32_t&>(l) ^= dir;;

	auto p0 = D2D1::Point2F(p1.x - l, p1.y - (c_arrow_h / 2.0f));
	auto p2 = D2D1::Point2F(p1.x - l, p1.y + (c_arrow_h / 2.0f));
	sink->BeginFigure(p0, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine(p1);
	sink->AddLine(p2);
	sink->AddLine(p0);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
}

void CLineBase::MakeArrowTipVertical(ATDIR dir, const D2D1::MyPoint2F& p1, ID2D1GeometrySink* sink) {
	float l = c_arrow_h;
	reinterpret_cast<uint32_t&>(l) ^= dir;;

	auto p0 = D2D1::Point2F(p1.x + (c_arrow_h / 2.0f), p1.y - l);
	auto p2 = D2D1::Point2F(p1.x - (c_arrow_h / 2.0f), p1.y - l);
	sink->BeginFigure(p0, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine(p1);
	sink->AddLine(p2);
	sink->AddLine(p0);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
}

std::unique_ptr<Command> CBlockLine::DeleteElement(CBlockComposition* pBlocks, CElement::MyQuadrantNode* qtRoot) {
	return std::make_unique<Command_DeleteLine>(this, pBlocks, qtRoot);
}


//Command_DeleteLine
Command_DeleteLine::Command_DeleteLine(CBlockLine* line, CBlockComposition* editor, CElement::MyQuadrantNode* qtRoot) 
	: line_(line), editor_(editor), qtRoot_(qtRoot) 
{
	line->SetSelect(false);
}

Command_DeleteLine::~Command_DeleteLine() {
	if (executed_) delete line_;
}

void Command_DeleteLine::Execute() {
	line_->Disconnect(editor_, qtRoot_);
	editor_->RemoveElement(line_);
	qtRoot_->DeleteElement(line_);
	executed_ = true;
}

void Command_DeleteLine::UnExecute() {
	line_->ConnectSlots(true, editor_, qtRoot_);
	editor_->Push(line_);
	qtRoot_->AddElement(line_);
	executed_ = false;
}