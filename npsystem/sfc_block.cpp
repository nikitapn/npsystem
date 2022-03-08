#include "stdafx.h"
#include "sfc_block.hpp"
#include "sfc_block_composition.hpp"
#include "sfc_block_factory.hpp"
#include "codegenerator.h"
#include <npsys/guid.h>
#include "graphics.h"
#include "constants.h"
#include "line.h"
#include "view_algorithm.h"
#include "command_impl.h"

__declspec(noinline) void cpp_sfc_line_delete::operator()(CSFCLine* line) {
	delete line;
}

// CSFCLine
NP_BOOST_CLASS_EXPORT_GUID(CSFCLine);
IMPLEMENT_VISITOR(CSFCLine);
// CSFCBus
IMPLEMENT_VISITOR(CSFCBusIn);
IMPLEMENT_VISITOR(CSFCBusOut);
// CSFCBlockBegin
NP_BOOST_CLASS_EXPORT_GUID(CSFCBlockBegin);
IMPLEMENT_VISITOR(CSFCBlockBegin);
IMPLEMENT_TRANSLATE_SFC(CSFCBlockBegin);
// CSFCBlockTerm
NP_BOOST_CLASS_EXPORT_GUID(CSFCBlockTerm);
IMPLEMENT_VISITOR(CSFCBlockTerm);
IMPLEMENT_TRANSLATE_SFC(CSFCBlockTerm);
// CSFCBlockStep
NP_BOOST_CLASS_EXPORT_GUID(CSFCBlockStep);
IMPLEMENT_VISITOR(CSFCBlockStep);
IMPLEMENT_TRANSLATE_SFC(CSFCBlockStep);
// CSFCBlockTransition
NP_BOOST_CLASS_EXPORT_GUID(CSFCBlockTransition);
IMPLEMENT_VISITOR(CSFCBlockTransition);
IMPLEMENT_TRANSLATE_SFC(CSFCBlockTransition);
// CSFCInputSlot
NP_BOOST_CLASS_EXPORT_GUID(CSFCInputSlot);
IMPLEMENT_VISITOR(CSFCInputSlot);
// CSFCOutputSlot
NP_BOOST_CLASS_EXPORT_GUID(CSFCOutputSlot);
IMPLEMENT_VISITOR(CSFCOutputSlot);
// CSFCBlockComposition
NP_BOOST_CLASS_EXPORT_GUID(CSFCBlockComposition);
IMPLEMENT_VISITOR(CSFCBlockComposition);

void CSFCLine::Recreate() {
	auto in = GetIn();
	auto out = GetOut();

	if (in->IsBusConnected() || out->IsBusConnected()) {
		// will be recreated from CSFCBus
	} else {
		Update();
		CreateSingle();
	}
}

void CSFCLine::CreatePredraw() {
	if (!IsGeometryDirty()) return;

	ID2D1GeometrySink* sink = nullptr;

	dis::Get().d2dFactory->CreatePathGeometry(m_geometry.ReleaseAndGetAddressOf());
	m_geometry->Open(&sink);

	auto &p1 = GetP1(), &p2 = GetP2();
	float dist = p2.x - p1.x;
	float arrow_dir = dist > 0 ? 1 : -1;
	const bool reverse_y_direction = (p2.y < p1.y - 5.0f) ? true : false;

	sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);

	float r_signed = c_line_r * arrow_dir;
	D2D1_SWEEP_DIRECTION arc_sweep_dir = dist > 0 ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE;

	if (reverse_y_direction) [[unlikely]]{
		float x3 = p1.x + 250.0f * arrow_dir;

		sink->AddLine(D2D1::Point2F(p1.x, p1.y + 20.0f - c_line_r));

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(p1.x + r_signed, p1.y + 20.0f),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(x3 - r_signed, p1.y + 20.0f)
		);

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(x3, p1.y + 20.0f - c_line_r),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(x3, p2.y + c_line_r)
		);

		float sign;
		CLineBase::ATDIR atdir;
		if (x3 > p2.x) {
			sign = -1.0f;
			arc_sweep_dir = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
			atdir = CLineBase::ATDIR::EW;
		} else {
			sign = 1.0f;
			arc_sweep_dir = D2D1_SWEEP_DIRECTION_CLOCKWISE;
			atdir = CLineBase::ATDIR::WE;
		}
		
		sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(x3 + c_line_r * sign, p2.y),
					D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
					D2D1_ARC_SIZE_SMALL)
			);

		sink->AddLine(
			D2D1::Point2F(p2.x - c_arrow_h * sign, p2.y)
		);

		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		MakeArrowTipHorizontal(atdir, p2, sink);
	} else { // forward y direction
		if (std::abs(dist) < c_arrow_h * 2.0f) [[unlikely]] { // almost straight
			sink->AddLine(D2D1::Point2F(p2.x, p2.y - c_arrow_h));
			sink->EndFigure(D2D1_FIGURE_END_OPEN);
		} else { // with middle point
			float y_mid = p1.y + (p2.y - p1.y) / 2;

			sink->AddLine(
				D2D1::Point2F(p1.x, y_mid - c_line_r)
			);

			sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + r_signed, y_mid),
					D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
					D2D1_ARC_SIZE_SMALL)
			);

			sink->AddLine(
				D2D1::Point2F(p2.x - r_signed, y_mid)
			);

			sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x, y_mid + c_line_r),
					D2D1::SizeF(c_line_r, c_line_r), 0, static_cast<D2D1_SWEEP_DIRECTION>(arc_sweep_dir ^ 1),
					D2D1_ARC_SIZE_SMALL)
			);

			sink->AddLine(
				D2D1::Point2F(p2.x, p2.y - c_arrow_h)
			);

			sink->EndFigure(D2D1_FIGURE_END_OPEN);
		}
		MakeArrowTipVertical(CLineBase::ATDIR::NS, p2, sink);
	}

	sink->Close();
	sink->Release();

	SetGeometryFlag(false);
	SetGeometryRealizationFlagDirty();
}

void CSFCLine::CreateSingle() {
	if (!IsGeometryDirty()) return;

	CalculateBounds();
	ID2D1GeometrySink* sink = nullptr;

	dis::Get().d2dFactory->CreatePathGeometry(m_geometry.ReleaseAndGetAddressOf());
	m_geometry->Open(&sink);

	auto &p1 = GetP1(), &p2 = GetP2();
	float dist = p2.x - p1.x;
	float arrow_dir = dist > 0 ? 1 : -1;
	const bool reverse_y_direction = (p2.y < p1.y - 5.0f) ? true : false;

	sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);

	float r_signed = c_line_r * arrow_dir;
	D2D1_SWEEP_DIRECTION arc_sweep_dir = dist > 0 ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE;

	if (reverse_y_direction) [[unlikely]] {
		float x3 = p1.x + 250.0f * arrow_dir;

		sink->AddLine(D2D1::Point2F(p1.x, p1.y + 20.0f - c_line_r));

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(p1.x + r_signed, p1.y + 20.0f),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(x3 - r_signed, p1.y + 20.0f)
		);

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(x3, p1.y + 20.0f - c_line_r),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(x3, p2.y - CSFCBus::c_offset_from_slot + c_line_r)
		);

		float sign;

		if (x3 > p2.x) {
			sign = -1.0f;
			arc_sweep_dir = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
		} else {
			sign = 1.0f;
			arc_sweep_dir = D2D1_SWEEP_DIRECTION_CLOCKWISE;
		}
		
		sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(x3 + c_line_r * sign,  p2.y - CSFCBus::c_offset_from_slot),
					D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
					D2D1_ARC_SIZE_SMALL)
			);

		sink->AddLine(
			D2D1::Point2F(p2.x - c_line_r * sign, p2.y - CSFCBus::c_offset_from_slot)
		);

		sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x,  p2.y - CSFCBus::c_offset_from_slot + c_line_r),
					D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
					D2D1_ARC_SIZE_SMALL)
			);

		sink->AddLine(
			D2D1::Point2F(p2.x, p2.y - c_arrow_h)
		);
	} else { // forward y direction
		if (std::abs(dist) < c_arrow_h * 2.0f) [[unlikely]] { // almost straight
			sink->AddLine(D2D1::Point2F(p2.x, p2.y - c_arrow_h));
		} else { // with middle point
			float y_mid = p1.y + (p2.y - p1.y) / 2;

			sink->AddLine(
				D2D1::Point2F(p1.x, y_mid - c_line_r)
			);

			sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p1.x + r_signed, y_mid),
					D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
					D2D1_ARC_SIZE_SMALL)
			);

			sink->AddLine(
				D2D1::Point2F(p2.x - r_signed, y_mid)
			);

			sink->AddArc(
				D2D1::ArcSegment(D2D1::Point2F(p2.x, y_mid + c_line_r),
					D2D1::SizeF(c_line_r, c_line_r), 0, static_cast<D2D1_SWEEP_DIRECTION>(arc_sweep_dir ^ 1),
					D2D1_ARC_SIZE_SMALL)
			);

			sink->AddLine(
				D2D1::Point2F(p2.x, p2.y - c_arrow_h)
			);
		}
	}

	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	MakeArrowTipVertical(CLineBase::ATDIR::NS, p2, sink);

	sink->Close();
	sink->Release();

	SetGeometryFlag(false);
	SetGeometryRealizationFlagDirty();
}

void CSFCLine::CreateForBus() {
	if (!IsGeometryDirty()) return;

	CalculateBounds();

	ID2D1GeometrySink* sink = nullptr;

	dis::Get().d2dFactory->CreatePathGeometry(m_geometry.ReleaseAndGetAddressOf());
	m_geometry->Open(&sink);

	auto &p1 = GetP1(), &p2 = GetP2();
	float dist = p2.x - p1.x;
	float arrow_dir = dist > 0 ? 1 : -1;
	const bool reverse_y_direction = (p2.y < p1.y - 5.0f) ? true : false;

	sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_HOLLOW);

	if (reverse_y_direction) [[unlikely]] {
		float r_signed = c_line_r * arrow_dir;
		D2D1_SWEEP_DIRECTION arc_sweep_dir = dist > 0 ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE;

		sink->AddLine(D2D1::Point2F(p1.x, p1.y + 20.0f - c_line_r));

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(p1.x + r_signed, p1.y + 20.0f),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(p2.x - r_signed, p1.y + 20.0f)
		);

		sink->AddArc(
			D2D1::ArcSegment(D2D1::Point2F(p2.x, p1.y + 20.0f - c_line_r),
				D2D1::SizeF(c_line_r, c_line_r), 0, arc_sweep_dir,
				D2D1_ARC_SIZE_SMALL)
		);

		sink->AddLine(
			D2D1::Point2F(p2.x, p2.y + c_arrow_h)
		);

		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		MakeArrowTipVertical(CLineBase::ATDIR::SN, p2, sink);
	} else { // forward y direction
		sink->AddLine(D2D1::Point2F(p2.x, p2.y - c_arrow_h));
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		MakeArrowTipVertical(CLineBase::ATDIR::NS, p2, sink);
	}

	sink->Close();
	sink->Release();

	SetGeometryFlag(false);
	SetGeometryRealizationFlagDirty();
}

// CSFCBlockFactory
CSFCBlock* CSFCBlockFactory::CreateBlock(int id) {
	switch (id) {
	case ID_CREATE_SFC_BLOCK_STEP:
		return CreateBlockStep();
	case ID_CREATE_SFC_BLOCK_TRANSITION:
		return CreateBlockTransition();
	case ID_CREATE_SFC_BLOCK_TERM:
		return CreateBlockTerm();
	default:
		ASSERT(FALSE);
		return nullptr;
	}
}

template<typename T>
void CSFCBlockFactory::CreateSlot(const D2D1::MyPoint2F& position_middle, CSFCBlock* block) {
	auto slot = new T(next_id());
	slot->Move(
		position_middle.x - constants::block::SLOT_SIZE / 2.0f,
		position_middle.y - constants::block::SLOT_SIZE / 2.0f
	);
	slot->SetParent(block);
	block->AddGraphicElement(slot);
}

CSFCBlockBegin* CSFCBlockFactory::CreateBlockBegin() {
	auto block = new CSFCBlockBegin();
	{
		auto text_name = new CTextElement(
			"BEGIN",
			constants::block::SFC_BLOCK_STEP_SIZE.width / 2.0f, 
			constants::block::SFC_BLOCK_STEP_SIZE.height / 2.0f, 
			CTextElement::T_CENTER | CTextElement::T_VCENTER,
			FONT_SIZE_CENTERED::SIZE_18
		);
		text_name->ResetHitAccess();
		block->elements_.push_back(text_name);
	}

	block->rect_.right = constants::block::SFC_BLOCK_STEP_SIZE.width;
	block->rect_.bottom = constants::block::SFC_BLOCK_STEP_SIZE.height;
	block->CreateRgnShape();

	CreateSlot<CSFCOutputSlot>({constants::block::SFC_BLOCK_STEP_SIZE.width / 2, constants::block::SFC_BLOCK_STEP_SIZE.height - constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);
	
	return block;
}

CSFCBlockTerm* CSFCBlockFactory::CreateBlockTerm() {
	auto block = new CSFCBlockTerm();

	block->rect_.right = constants::block::SFC_BLOCK_TERM_SIZE.width;
	block->rect_.bottom = constants::block::SFC_BLOCK_TERM_SIZE.height;
	block->CreateRgnShape();

	CreateSlot<CSFCInputSlot>({constants::block::SFC_BLOCK_TERM_SIZE.width / 2, constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);
	
	return block;
}

CSFCBlockStep* CSFCBlockFactory::CreateBlockStep() {
	auto block = new CSFCBlockStep();
	{
		auto text_name = new CTextElement(
			blocks_->CreateUniqueName(block->GetTypeNameStr()),
			constants::block::SFC_BLOCK_STEP_SIZE.width / 2.0f, 
			constants::block::SFC_BLOCK_STEP_SIZE.height / 2.0f, 
			CTextElement::T_CENTER | CTextElement::T_VCENTER,
			FONT_SIZE_CENTERED::SIZE_18
		);
		text_name->ResetHitAccess();
		block->elements_.push_back(text_name);
	}

	block->rect_.right = constants::block::SFC_BLOCK_STEP_SIZE.width;
	block->rect_.bottom = constants::block::SFC_BLOCK_STEP_SIZE.height;
	block->CreateRgnShape();

	CreateSlot<CSFCInputSlot>({constants::block::SFC_BLOCK_STEP_SIZE.width / 2, constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);
	CreateSlot<CSFCOutputSlot>({constants::block::SFC_BLOCK_STEP_SIZE.width / 2, constants::block::SFC_BLOCK_STEP_SIZE.height - constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);

	return block;
}

CSFCBlockTransition* CSFCBlockFactory::CreateBlockTransition() {
	auto block = new CSFCBlockTransition();
	{
		auto text_name = new CTextElement(
			blocks_->CreateUniqueName(block->GetTypeNameStr()),
			constants::block::SFC_BLOCK_STEP_SIZE.width / 2.0f, 
			constants::block::SFC_BLOCK_STEP_SIZE.height / 2.0f, 
			CTextElement::T_CENTER | CTextElement::T_VCENTER,
			FONT_SIZE_CENTERED::SIZE_12
		);
		text_name->ResetHitAccess();
		block->elements_.push_back(text_name);
	}
	block->rect_.right = constants::block::SFC_BLOCK_TRANSITION_SIZE.width;
	block->rect_.bottom = constants::block::SFC_BLOCK_TRANSITION_SIZE.height;
	block->CreateRgnShape();

	CreateSlot<CSFCInputSlot>({constants::block::SFC_BLOCK_TRANSITION_SIZE.width / 2, constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);
	CreateSlot<CSFCOutputSlot>({constants::block::SFC_BLOCK_TRANSITION_SIZE.width / 2, constants::block::SFC_BLOCK_TRANSITION_SIZE.height - constants::block::SFC_SLOT_CIRCLE_RADIUS}, block);

	return block;
}

// CSFCBlock

void CSFCBlock::Draw(CGraphics* graphics) {
	auto const normalMode = !IsEditState();
	auto const colorStroke = (IsSelected() ? SolidColor::SelColor : (IsHover() ? SolidColor::SliderThingHovered : m_colorStroke));

	if (normalMode && IsGeometryRealizationDirty(graphics->GetId())) {
		graphics->CreateFilledGeometryRealization(
			m_geometryTransformed.Get(), m_geometryRealizationFilled.ReleaseAndGetAddressOf());
		graphics->CreateStrokedGeometryRealization(
			m_geometryTransformed.Get(), 1.0f, m_geometryRealizationStroked.ReleaseAndGetAddressOf());
		SetGeometryRealizationFlagAfterDraw(graphics->GetId());
	}

	if (normalMode) [[likely]] {
		graphics->DrawBlockGeometry(rect_, m_geometryRealizationFilled.Get(), BlockColor::SfcBlockStep);
		if (colorStroke != SolidColor::NoColor) {
			graphics->DrawGeometryRealization(m_geometryRealizationStroked.Get(), colorStroke);
		}
	} else {
		if (m_colorFill != SolidColor::NoColor) [[likely]] {
			graphics->FillGeometry(m_geometryTransformed.Get(), m_colorFill);
		}
			if (colorStroke != SolidColor::NoColor) {
				graphics->DrawGeometry(m_geometryTransformed.Get(), colorStroke);
			}
	}
}

// CreateRgnShape()

void CSFCBlockBegin::CreateRgnShape() noexcept {
	auto size = GetSize();
	if (size == constants::block::SFC_BLOCK_STEP_SIZE) [[likely]] {
		m_geometry = dis::Get().sfc_begin_block;
	} else {
		m_geometry = dis::Get().CreateSFCBeginBlock(size);
	}
}

void CSFCBlockTerm::CreateRgnShape() noexcept {
	auto size = GetSize();
	if (size == constants::block::SFC_BLOCK_TERM_SIZE) [[likely]] {
		m_geometry = dis::Get().sfc_term_block;
	} else {
		m_geometry = dis::Get().CreateSFCTermBlock(size);
	}
}

void CSFCBlockStep::CreateRgnShape() noexcept {
	auto size = GetSize();
	if (size == constants::block::SFC_BLOCK_STEP_SIZE) [[likely]] {
		m_geometry = dis::Get().sfc_step_block;
	} else {
		m_geometry = dis::Get().CreateSFCStepBlock(size);
	}
}

void CSFCBlockTransition::CreateRgnShape() noexcept {
	m_geometry = dis::Get().sfc_transition_block;
}

bool CSFCBlockStep::SetFarCorner(float x, float y) {
	bool succeded = CElement::SetFarCorner(x, y);
	if (succeded) {
		CreateRgnShape();
		UpdateTransformedGeometry();
	}
	return succeded;
}

bool CSFCBlockBegin::ConnectToLine(CSFCLine* line, bool checking) {
	return line->GetOut() ? false : GetOutputSlot()->ConnectToLine(line, checking);
}

bool CSFCBlockTerm::ConnectToLine(CSFCLine* line, bool checking) {
	return line->GetIn() ? false : GetInputSlot()->ConnectToLine(line, checking);
}

bool CSFCBlock2::ConnectToLine(CSFCLine* line, bool checking) {
	return line->GetOut() ? GetInputSlot()->ConnectToLine(line, checking) : GetOutputSlot()->ConnectToLine(line, checking);
}

void CSFCBlock2::SetName(const std::string& name) noexcept {
	GetTextName()->SetName(name);
}

const std::string& CSFCBlock2::GetName() const noexcept {
	return GetTextName()->GetName();
}

// CSFCBus 

void CSFCBusIn::CreateArrow(ID2D1GeometrySink* sink) {
	sink->BeginFigure({x_middle_, y_}, D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine({x_middle_, y_ + 20.0f - c_arrow_h});
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	MakeArrowTipVertical(ATDIR::NS, {x_middle_, y_ + 20.0f}, sink);
}

void CSFCBusOut::CreateArrow(ID2D1GeometrySink* sink) {
	sink->BeginFigure({x_middle_, y_ - 20.0f}, D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine({x_middle_, y_ - c_arrow_h});
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	MakeArrowTipVertical(ATDIR::NS, {x_middle_, y_}, sink);
}

void CSFCBus::Create(float x1, float x2) {
	if (x1 > x_middle_) x1 = x_middle_;
	if (x2 < x_middle_) x2 = x_middle_;

	x1 -= c_extra_width;
	x2 += c_extra_width;

	rect_.left = x1;
	rect_.right = x2;
	
	rect_.top = y_ - c_hl;
	rect_.bottom = y_ + c_hl;

	
	dis::Get().d2dFactory->CreatePathGeometry(m_geometry.ReleaseAndGetAddressOf());

	ID2D1GeometrySink* sink = nullptr;
	m_geometry->Open(&sink);

	// bus
	sink->BeginFigure(GetCorner0(), D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine({x1, y_ + c_hl});
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->BeginFigure({x1, y_}, D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine({x2, y_});
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->BeginFigure({x2, y_ - c_hl} , D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLine(GetCorner1());
	sink->EndFigure(D2D1_FIGURE_END_OPEN);

	CreateArrow(sink);

	sink->Close();
	sink->Release();

	SetGeometryRealizationFlagDirty();
}	
template<typename... T>
void CSFCBus::Connect(T*... lines) {
	(lines_.push_back(lines), ...);
}

void CSFCBusIn::CalcLinesPosition() {
		std::sort(lines_.begin(), lines_.end(), [](CSFCLine* l1, CSFCLine* l2) {
			return l1->GetP1().x < l2->GetP1().x;
			});

		for (auto& l : lines_) {
			auto const pt = l->GetP1();
			if (pt.y < y_) {
				l->SetP2({pt.x, y_-1.5f});
			} else {
				auto dx = 2.5f + l->GetIn()->GetParentBlock()->GetSize().width / 2.0f;
				if (pt.x > x_middle_) {
					l->SetP2({pt.x - dx, y_+1.5f});
				} else {
					l->SetP2({pt.x + dx, y_+1.5f});
				}
			}
			l->CreateForBus();
		}
		Create(lines_[0]->GetP2().x, lines_.back()->GetP2().x);
}

void CSFCBusOut::CalcLinesPosition() {
	std::sort(lines_.begin(), lines_.end(), [](CSFCLine* l1, CSFCLine* l2) {
		return l1->GetP2().x < l2->GetP2().x;
		});

	for (auto& l : lines_) {
		l->SetP1({l->GetP2().x, y_});
		l->CreateForBus();
	}

	Create(lines_[0]->GetP2().x, lines_.back()->GetP2().x);
}

// CSFCSlot

CSFCSlot::CSFCSlot()
{
	SetZOrder(2);
	SetHitAccess();
	m_geometry = dis::Get().geometry_slot;
}

CSFCSlot::CSFCSlot(int id)
	: id_{id}
{
	SetZOrder(2);
	SetHitAccess();
	m_geometry = dis::Get().geometry_slot;
	rect_ = {0, 0, constants::block::SLOT_SIZE, constants::block::SLOT_SIZE};
}

void CSFCSlot::Draw(CGraphics* graphics) {
	D2D1_ELLIPSE el;
	el.point = GetMiddle();
	el.radiusX = constants::block::SLOT_SIZE;
	el.radiusY = constants::block::SLOT_SIZE;

	if (IsHover()) {
		graphics->FillSlotFocused(el);
	} else if (IsConnected()) {
		graphics->FillSlotConnected(el);
	} else {
		graphics->FillSlot(el);
	}
}

void CSFCSlot::Move(float x, float y) {
	__super::Move(x, y);
	UpdateLineOrBus();
}

void CSFCSlot::MoveDxDy(float dx, float dy) {
	__super::MoveDxDy(dx, dy);
	UpdateLineOrBus();
}

void CSFCOutputSlot::UpdateLineOrBus() noexcept {
	if (IsBusConnected()) {
		UpdateBusPosition();
	} else if (line_) {
		UpdateLinePosition();
	}
}

void CSFCInputSlot::UpdateLineOrBus() noexcept {
	if (IsBusConnected()) {
		UpdateBusPosition();
	} else if (line_) {
		UpdateLinePosition();
	}
}

CSFCLine* CSFCBus::DisconnectLine(CSFCLine* line) {
	auto it = std::find(lines_.begin(), lines_.end(), line);
	ASSERT(it != lines_.end());
	lines_.erase(it);
	if (lines_.size() == 1) {
		return lines_[0];
	} else {
		CalcLinesPosition();
		return nullptr;
	}
}

void CSFCSlot::DisconnectLine(CSFCLine* line, CSFCBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot) {
	ASSERT(IsConnected());
	
	if (!IsBusConnected()) {
		line_ = nullptr;
		return;
	}

	if (auto last_line = bus_->DisconnectLine(line); last_line) {
		qtRoot->DeleteElement(last_line);
		line_ = last_line;
		line_->Update();
		line_->CreateSingle();
		qtRoot->AddElement(last_line);
		
		ASSERT(bus_->LinesCount() == 1);
		bus_->Clear();
		DisconnectBus(blocks);
		qtRoot->DeleteElement(bus_);
		bus_ = nullptr;
	}
}

bool CSFCInputSlot::ConnectToLine(CSFCLine* line, bool checking) {
	ASSERT(line);
	if (checking) {
		if (IsConnected() && line->GetOut()->IsConnected()) { return false; }
		if (IsConnected() && GetLine()->GetOut()->GetBus()) { return false; }
		if (line->GetOut()->IsConnected() && line->GetOut()->GetLine()->GetIn()->GetBus()) { return false; }
	}
	return line->SetSlot(this);
}

bool CSFCOutputSlot::ConnectToLine(CSFCLine* line, bool checking) {
	ASSERT(line);
	if (checking) {
		if (IsConnected() && line->GetIn()->IsConnected()) { return false; }
		if (IsConnected() && line->GetIn()->GetBus()) { return false; }
		if (line->GetIn()->IsConnected() && line->GetIn()->GetLine()->GetOut()->GetBus()) { return false; }
	}
	return line->SetSlot(this);
}

template<typename T>
void CSFCSlot::ConnectLine(CSFCLine* line, CSFCBlockComposition* blocks) {
	if (IsConnected()) {
		if (IsBusConnected()) {
			bus_->Connect(line);
		} else {
			ASSERT(line_);
			if (!bus_) bus_ = blocks->GetBus<T>();
			bus_->Connect(line_, line);
			UpdateBusPosition();
		}
	} else {
		line_ = line;
	}
}

void CSFCLine::ConnectSlots(bool recreate_dependencies, CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot) {
	ASSERT(blocks);

	auto in = GetIn();
	auto out = GetOut();

	ASSERT(!(in->GetBus() && out->GetBus()));

	int action =
		(in->IsConnected() << 0) |
		(in->IsBusConnected() << 1) |
		(out->IsConnected() << 2) |
		(out->IsBusConnected() << 3);

	in->ConnectLine<CSFCBusIn>(this, static_cast<CSFCBlockComposition*>(blocks));
	out->ConnectLine<CSFCBusOut>(this, static_cast<CSFCBlockComposition*>(blocks));

	if (!recreate_dependencies) return;
	
	CSFCBus* bus = nullptr;
	
	switch (action) 
	{
	case 0:
		Update();
		CreateSingle();
		break;
	case ((1 << 0) | (1 << 1)): // input slot already has had bus before
	case (1 << 0): // input slot now has bus
		out->UpdateLinePosition();
		bus = in->GetBus();
		break;
	case ((1 << 2) | (1 << 3)): // output slot already has had bus before
	case (1 << 2): // output slot now has bus
		in->UpdateLinePosition();
		bus = out->GetBus();
		break;
	default:
		ASSERT(FALSE);
	}

	if (bus) {
		qtRoot->DeleteElement(bus);
		bus->Recreate();
		qtRoot->AddElement(bus);
	}
}

void CSFCLine::Disconnect(CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot) {
	ASSERT(slot_in_ && slot_out_);
	slot_in_->DisconnectLine(this, static_cast<CSFCBlockComposition*>(blocks), qtRoot);
	slot_out_->DisconnectLine(this, static_cast<CSFCBlockComposition*>(blocks), qtRoot);
}

void CSFCInputSlot::UpdateLinePosition() {
	ASSERT(line_);
	auto middle = GetMiddle();
	line_->SetP2({middle.x, GetParentBlock()->GetBounds().top - 1.5f});
}

void CSFCOutputSlot::UpdateLinePosition() {
	ASSERT(line_);
	auto middle = GetMiddle();
	line_->SetP1({middle.x, GetParentBlock()->GetBounds().bottom + 1.5f});
}

void CSFCInputSlot::UpdateBusPosition() {
	auto middle = GetMiddle();
	const auto y = GetParentBlock()->GetBounds().top - 1.5f - CSFCBus::c_offset_from_slot;
	bus_->SetPosition(middle.x, y);
}

void CSFCOutputSlot::UpdateBusPosition() {
	auto middle = GetMiddle();
	const auto y = GetParentBlock()->GetBounds().bottom + 1.5f + CSFCBus::c_offset_from_slot;
	bus_->SetPosition(middle.x, y);
}

void CSFCInputSlot::DisconnectBus(CSFCBlockComposition* blocks) {
	blocks->FreeBus(static_cast<CSFCBusIn*>(bus_));
}

void CSFCOutputSlot::DisconnectBus(CSFCBlockComposition* blocks) {
	blocks->FreeBus(static_cast<CSFCBusOut*>(bus_));
}

void CSFCInputSlot::CollectGeometries(std::unordered_set<IRecreatable*>& lines) {
	if (!IsConnected()) return;
	if (IsBusConnected()) { lines.emplace(GetBus()); return; }
	if (GetLine()->GetOut()->IsBusConnected()) { lines.emplace(GetLine()->GetOut()->GetBus()); return; }

	lines.emplace(GetLine());
}

void CSFCOutputSlot::CollectGeometries(std::unordered_set<IRecreatable*>& lines) {
	if (!IsConnected()) return;
	if (IsBusConnected()) { lines.emplace(GetBus()); return; }
	if (GetLine()->GetIn()->IsBusConnected()) { lines.emplace(GetLine()->GetIn()->GetBus()); return; }

	lines.emplace(GetLine());
}

void CSFCBlock::CollectLines(CSFCSlot* slot, boost::container::small_vector<CSFCLine*, 8>& lines) {
	if (!slot->IsConnected()) return;

	if (slot->IsBusConnected()) return slot->GetBus()->GetLines(lines);
	
	lines.push_back(slot->GetLine());
}

// CSFCSlotManipulator

S_CURSOR CSFCSlot::CSFCSlotManipulator::MouseButtonDown(CPoint point, CAlgorithmView* wnd) {
	line_.reset(new CSFCLine);

	auto middle = target_->GetMiddle();
	begin_at_ = target_->GetDirection();

	line_->SetP1(middle);
	line_->SetP2(middle);

	if (begin_at_ == SlotDirection::INPUT_SLOT) {
		line_->SetSlot(static_cast<CSFCInputSlot*>(target_));
	} else {
		line_->SetSlot(static_cast<CSFCOutputSlot*>(target_));
	}

	fs_.clear();
	TraversalExclude<CBlockVisitor>(static_cast<CSFCView*>(wnd)->GetEditor(), target_->GetParentBlock(), fs_);

	return S_CURSOR::A_LINE;
}

int CSFCSlot::CSFCSlotManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	CGraphics* pGraphics = pWnd->GetGraphics();
	const D2D1_MATRIX_3X2_F& invM = pGraphics->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pGraphics->GetMatrix();

	D2D1::MyPoint2F pt = D2D1::MyPoint2F((float)point.x, (float)point.y) * invM;
	
	if (begin_at_ == SlotDirection::INPUT_SLOT) line_->SetP1(pt);
	else line_->SetP2(pt);

	for (auto sl : fs_) {
		UINT elflags;
		if (sl->HitTestWhileConnectingLine(pt, elflags)) break; // updating slot hover state
	}

	return 2;
}

void CSFCSlot::CSFCSlotManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	CGraphics* pGraphics = pWnd->GetGraphics();
	const D2D1_MATRIX_3X2_F& invM = pGraphics->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pGraphics->GetMatrix();

	target_->SetHover(false);
	
	D2D1::MyPoint2F pt = D2D1::MyPoint2F((float)point.x, (float)point.y) * invM;
	for (auto& i : fs_) {
		UINT elflags;
		if (i->HitTestWhileConnectingLine(pt, elflags)) {
			if (line_->SetSlot(i, true)) {
				pWnd->SetCursor(NPSystemCursor::Arrow);
				pWnd->Insert(line_.release());
				return;
			}
			MessageBoxA(pWnd->m_hWnd, "Wrong operation...", "Error", 0);
			break;
		}
	}

	line_.reset();

	pWnd->SetCursor(NPSystemCursor::Arrow);
	pWnd->Invalidate();
}

void CSFCSlot::CSFCSlotManipulator::Draw(CGraphics* pGraphics) {
	line_->CreatePredraw();
	line_->PreDraw(pGraphics);
}

CSFCBlockComposition::CSFCBlockComposition(std::true_type)
	: block_factory_(this)
{
	auto block = block_factory_.CreateBlockBegin();
	block->Move(256.0f, 0);
	Push(block);
}

// CSFCBlockManipulator


int CSFCBlock::CSFCBlockManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	auto result = CElementManipulator::Drag(point, pWnd);

	if (result == 1) {
		geometries_.clear();
		static_cast<CSFCBlock*>(element_)->CollectAffectedGeometries(geometries_);
		for (auto& g : geometries_) pWnd->qtRoot.DeleteElement(g->GetBase());
	} else if (result == 2) {
		for (auto& g : geometries_) g->Recreate();
	}

	return result;
}

void CSFCBlock::CSFCBlockManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	if (!was_draged_) return;
	for (auto& l : geometries_) pWnd->qtRoot.AddElement(l->GetBase());
	CElementManipulator::MouseButtonUP(point, pWnd);
}

void CSFCBlock::CSFCBlockManipulator::Draw(CGraphics* graphics) {
	if (!was_draged_) return;
	element_->Draw(graphics);
	for (auto& l : geometries_) l->GetBase()->Draw(graphics);
}

class SFCCommand_BlockMove
	: public Command_MoveCommand {
protected:
	std::unordered_set<IRecreatable*> affected_;

	void RecreateLines() {
		for (auto a : affected_) {
			qtRoot_->DeleteElement(a->GetBase());
			a->Recreate();
			qtRoot_->AddElement(a->GetBase());
		}
	}
public:
	virtual void Execute() {
		Command_MoveCommand::Execute();
		RecreateLines();
	}

	virtual void UnExecute() {
		Command_MoveCommand::UnExecute();
		RecreateLines();
	}

	SFCCommand_BlockMove(CSFCBlock* block, const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) 
		: Command_MoveCommand(block, delta, qtRoot) 
	{
		block->CollectAffectedGeometries(affected_);
	}
};

std::unique_ptr<Command_MoveCommand> CSFCBlock::CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) {
	return std::make_unique<SFCCommand_BlockMove>(this, delta, qtRoot);
}

std::unique_ptr<Command> CSFCBlock::DeleteElement(CBlockComposition* blocks, MyQuadrantNode* qtRoot) {
	class DeleteSFCBlockCommand : public Command {
		CSFCBlock* block_;
		CBlockComposition* editor_;
		CElement::MyQuadrantNode* qtRoot_;
	public:
		virtual void Execute() {
			RemoveFromQuadrant(block_, qtRoot_);
			editor_->RemoveElement(block_);
			executed_ = true;
		}

		virtual void UnExecute() {
			PasteToQuadrant(block_, qtRoot_);
			editor_->Push(block_);
			executed_ = false;
		}

		virtual void ExecutePermanent() {
			block_->DeletePermanent();
			delete block_;
		}

		DeleteSFCBlockCommand(CSFCBlock* block, CBlockComposition* editor, CElement::MyQuadrantNode* qtRoot)
			: block_(block), editor_(editor), qtRoot_(qtRoot) 
		{
			block->SetSelect(false);
		}
	};

	auto cmd = std::make_unique<GroupCommand<Command>>();
	for (auto& l : GetLines()) cmd->CreateCommand<Command_DeleteLine>(true, l, blocks, qtRoot);
	cmd->CreateCommand<DeleteSFCBlockCommand>(false, this, blocks, qtRoot);
	return cmd;
}

void CSFCBlock::DeletePermanent() {

}

void CSFCSlot::PrintDebugTooltip(std::stringstream& ss) {
	ss << "connected: " << std::boolalpha << IsConnected() << "\n"
	<< "bus_connected: " << std::boolalpha << IsBusConnected();
}

__declspec(noinline) void cpp_delete_CSFCBusIn::operator()(CSFCBusIn* ptr) { delete ptr; }
__declspec(noinline) void cpp_delete_CSFCBusOut::operator()(CSFCBusOut* ptr) { delete ptr; };


void CBlockCompositionBase::SFC_RestorePointers() {
	CSFCFindSlots slots;
	CFindType<CSFCLine, CBlockVisitor> lines;
	
	Traversal<CBlockVisitor>(this, slots, lines);
	// Restore lines
	ConnectLinesToSlotsAfterDeserialization(lines.get(), slots.get());
	
	for (auto l : lines) l->Update();

	std::unordered_set<IRecreatable*> geometries;
	for (auto s : slots) s->CollectGeometries(geometries);
	for (auto g : geometries) g->Recreate();
}