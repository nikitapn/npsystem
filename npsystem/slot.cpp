// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "block.h"
#include "line.h"
#include "view_algorithm.h"
#include "codegenerator.h"
#include "graphics.h"
#include "constants.h"
#include "algext.h"
#include "assignedalgorithm.h"
#include <npsys/algsubcat.h>
#include <npsys/fbdblock.h>
#include "io_avr.h"
#include "network_ext.h"
#include "avrassigned.h"
#include "module.h"
#include <npsys/guid.h>
#include <npsys/strings.h>

__declspec(noinline) void cpp_line_delete::operator()(CLine* line) {
	delete line;
}

static const std::string _invalid_link_ = "<INVALID LINK>";
static const std::string _invalid_external_link_ = "<INVALID EXTERNAL LINK>";
static const std::string _invalid_peripheral_link_ = "<INVALID PERIPHERAL LINK>";
// CSlotGroup
NP_BOOST_CLASS_EXPORT_GUID(CSlotGroup);
IMPLEMENT_VISITOR(CSlotGroup);
// CInputSlot
NP_BOOST_CLASS_EXPORT_GUID(CInputSlot);
IMPLEMENT_VISITOR(CInputSlot);
// COutputSlot
NP_BOOST_CLASS_EXPORT_GUID(COutputSlot);
IMPLEMENT_VISITOR(COutputSlot);
// CValue
NP_BOOST_CLASS_EXPORT_GUID(CValue);
IMPLEMENT_VISITOR(CValue);
// CBlockInput
NP_BOOST_CLASS_EXPORT_GUID(CBlockInput);
IMPLEMENT_VISITOR(CBlockInput);
// CInternalRef
NP_BOOST_CLASS_EXPORT_GUID(CInternalRef);
IMPLEMENT_VISITOR(CInternalRef);
// ValueRef
NP_BOOST_CLASS_EXPORT_GUID(CValueRef);
IMPLEMENT_VISITOR(CValueRef);
// CFixedValue
NP_BOOST_CLASS_EXPORT_GUID(CFixedValue);
IMPLEMENT_VISITOR(CFixedValue);
// CAvrInternalPin
NP_BOOST_CLASS_EXPORT_GUID(CAvrInternalPin);
IMPLEMENT_VISITOR(CAvrInternalPin);
// CModuleValue
NP_BOOST_CLASS_EXPORT_GUID(CModuleValue);
IMPLEMENT_VISITOR(CModuleValue);
// CExternalReference
NP_BOOST_CLASS_EXPORT_GUID(CExternalReference);
IMPLEMENT_VISITOR(CExternalReference);

// CSlot

CSlot::CSlot()
	: manipulator_(this) {
	SetZOrder(2);
	SetHitAccess();
}

CSlot::CSlot(CSlot& slot)
	: CGeometryElement(slot)
	, manipulator_(this) {
	SetHitAccess();
}

CSlot::CSlot(int id, CSlotType* slot_type)
	: manipulator_(this)
	, id_(id)
	, slot_type_(slot_type)
	, parent_(nullptr)
	, slot_type_saved_(nullptr)
{
	SetZOrder(2);
	SetHitAccess();
}

CSlot::~CSlot() {
	// 	sig_state_chaged_.disconnect_all_slots();
	//	std::cout << "CSlot::~CSlot()" << std::endl;
}

CElement::_Iterator* CSlot::CreateIterator() {
	iterator_.Set(&text_);
	return &iterator_;
}

void CSlot::Move(float x, float y) {
	__super::Move(x, y);
	AlignText();
}

void CSlot::MoveDxDy(float dx, float dy) {
	__super::MoveDxDy(dx, dy);
	text_.MoveDxDy(dx, dy);
}

void CSlot::Draw(CGraphics* graphics) {
	D2D1_ELLIPSE el;
	el.point = GetMiddle();
	el.radiusX = constants::block::SLOT_SIZE;
	el.radiusY = constants::block::SLOT_SIZE;
	if (activated_) {
		graphics->FillSlotActivated(el);
	} else if (IsHover()) {
		graphics->FillSlotFocused(el);
	} else if (m_bHasLines) {
		graphics->FillSlotConnected(el);
	} else {
		graphics->FillSlot(el);
	}
}

void CSlot::PrintDebugTooltip(std::stringstream& ss) {
	ss << "slot_id: " << top->self_node.id() << ", parent_block_id: " << top->block_parent.id() << '\n';
	GetSlotType()->PrintDebugTooltip(ss);
}

void CSlot::CreateRgnShape() noexcept {
	dis::Get().d2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, constants::block::SLOT_SIZE, constants::block::SLOT_SIZE),
		(ID2D1RectangleGeometry**)m_geometry.GetAddressOf());
}

void CSlot::SetSlotType(CSlotType* slot_type) {
	if (slot_type->Equal(slot_type_.get())) {
		delete slot_type;
		return;
	}
	if (!slot_type_saved_) {
		slot_type_saved_ = std::move(slot_type_);
		slot_type_.reset(slot_type);
	} else {
		if (slot_type_saved_->Equal(slot_type)) {
			delete slot_type;
			slot_type_ = std::move(slot_type_saved_);
		} else {
			slot_type_.reset(slot_type);
		}
	}
}

void CSlot::BeforeStartOnline() {
	auto var = GetVariable();
	if (var && var->GetClearType() == npsys::variable::VT_BYTE && !top->strings.is_invalid_id()) {
		Online.string = top->strings.fetch();
	}
	SetValue(nullptr);
}
void CSlot::AfterStopOnline() {
	Online.string = {};
	SetValue(nullptr);
}

void CSlot::SetValue(const nps::server_value* value) {
	activated_ = false;

	if (value != nullptr) {
		value_ = *value;
		_stValue = value_.to_string(3);
		auto has_quality = !npsys::variable::IsIO(value_.get_type()) && npsys::variable::IsQuality(value_.get_type());
		if (value_.is_discrete()) {
			if (has_quality) {
				const auto& qbit = value_.to_Qbit();
				activated_ = (qbit.value == NPSYSTEM_TRUE);
			} else {
				const auto& bit = value_.to_bit();
				activated_ = (bit.value == NPSYSTEM_TRUE);
			}
		}

		if (_stValue == "x") {
			Online.text_layout = dis::Get().text_layout_bad_quality.Get();
		} else {
			std::wstring str;
			if (Online.string.loaded()) {
				size_t index = 0;
				if (value_.is_quality()) index = value_.to_Qu8().value;
				else index = value_.to_u8().value;

				auto name = Online.string->get(index);
				if (name) str = wide(*name);
				else str = wide("unknown name: " + _stValue);
			} else {
				str = wide(_stValue);
			}
			HR(dis::Get().pDWriteFactory->CreateTextLayout(str.c_str(), static_cast<int>(str.length()),
				dis::Get().pTextOnline,
				100, 18,
				Online.text_layout.ReleaseAndGetAddressOf()));
		}
	} else {
		Online.text_layout = dis::Get().text_layout_unknown.Get();
	}

	AlignOnlineValue();
}

void CSlot::DrawOnlineValue(CGraphics* pGraphics) {
	if (!Online.text_layout) return;

	pGraphics->DrawValueBk(Online.rc);
	pGraphics->DrawTextLayout(Online.pt, Online.text_layout.Get(), SolidColor::Black);
}

void CSlot::CommitTypeChanges() {
	if (slot_type_saved_) {
		slot_type_saved_->ReleaseMemory();
		slot_type_saved_.reset();
	}
}

std::string CSlot::GetFullName() {
	if (parent_) {
		return parent_->GetName() + "." + GetName();
	} else {
		auto block = top->block_parent.fetch();
		return block->e_block->GetName() + "." + GetName();
	}
}

void CSlot::UpdateSlotPosition() noexcept {
	CreateRgnShape();
	m_matrixTransform = D2D1::Matrix3x2F::Translation(rect_.left, rect_.top);
	AlignText();
	dis::Get().d2dFactory->CreateTransformedGeometry(m_geometry.Get(), m_matrixTransform,
		m_geometryTransformed.ReleaseAndGetAddressOf());
}

void CSlot::SetSlotPosition(size_t index, float x, float y, float block_width) noexcept {
	index_ = index;

	auto input_slot_rect = [](const int position, float x, float y) noexcept {
		return D2D1::RectF(
			x + constants::block::SLOT_SIZE,
			y + constants::block::y_pos(position),
			x + 20.0f,
			y +constants::block::y_pos(position) + constants::block::SLOT_SIZE
		);
	};

	auto output_slot_rect = [block_width](const int position, float x, float y) noexcept {
		return D2D1::RectF(
			x + block_width - 20.0f,
			y + constants::block::y_pos(position),
			x + block_width - 10.0f,
			y + constants::block::y_pos(position) + constants::block::SLOT_SIZE
		);
	};

	rect_ = (GetDirection() == INPUT_SLOT)
		? input_slot_rect(static_cast<int>(index), x, y)
		: output_slot_rect(static_cast<int>(index), x, y);

	UpdateSlotPosition();
}

void CSlot::SetInputSlotPosition(const D2D1_RECT_F& block_rect) noexcept {
	index_ = 0;

	rect_.left = block_rect.left + constants::block::SLOT_SPACE;
	rect_.right = rect_.left + constants::block::SLOT_SIZE;
	rect_.top = block_rect.top + constants::block::SLOT_SPACE;
	rect_.bottom = rect_.top + constants::block::SLOT_SIZE;

	UpdateSlotPosition();
}

void CSlot::SetOutputSlotPosition(const D2D1_RECT_F& block_rect) noexcept {
	index_ = 0;

	rect_.left = block_rect.right - constants::block::SLOT_SIZE - constants::block::SLOT_SPACE;
	rect_.right = rect_.left + constants::block::SLOT_SIZE;
	rect_.top = block_rect.top + constants::block::SLOT_SPACE;
	rect_.bottom = rect_.top + constants::block::SLOT_SIZE;

	UpdateSlotPosition();
}

// CInputSlot

bool CInputSlot::ConnectToLine(CLine* line) {
	ATLASSERT(line);
	return lines_.size() != 0 ? false : line->SetInputSlot(this);
}

void CInputSlot::AddLine(CLine* line) {
	ATLASSERT(line);
	ATLASSUME(lines_.size() == 0);
	lines_.push_back(line);
	m_bHasLines = TRUE;
}

void CInputSlot::DisconnectLine(CLine* line) {
	ASSERT(lines_.size() == 1);
	lines_.clear();
	m_bHasLines = FALSE;
}

int CInputSlot::IsConnected() {
	return (lines_.size() == 1);
}

CSlot* CInputSlot::GetAnotherSlot() {
	return lines_.size() == 0 ? nullptr : lines_[0]->GetAnotherSlot(this);
}

npsys::variable* CInputSlot::GetInputVariable() {
	auto in = GetAnotherSlot();
	if (in) return in->GetVariable();
	return nullptr;
}

npsys::variable* CInputSlot::GetSlotAssociateVariable() {
	auto var = GetVariable();
	if (var) return var;
	return GetInputVariable();
}

int CInputSlot::DefineTypeR() {
	return IsConnected()
		? lines_[0]->GetAnotherSlot(this)->DefineTypeR()
		: npsys::variable::VT_UNDEFINE;
}


void CInputSlot::AlignOnlineValue() {
	DWRITE_TEXT_METRICS metrics;
	Online.text_layout->GetMetrics(&metrics);
	Online.rc.rect.left = metrics.left + rect_.left - 15 - metrics.width;
	Online.rc.rect.top = metrics.top + rect_.top - 20;
	Online.rc.rect.right = Online.rc.rect.left + metrics.width;
	Online.rc.rect.bottom = Online.rc.rect.top + metrics.height;
	Online.rc.radiusX = 1;
	Online.rc.radiusY = 1;
}

void CInputSlot::AlignText() {
	D2D1::MyPoint2F pt = GetMiddle();
	text_.SetPosition(pt.x + 10, pt.y - 0.5f, CTextElement::T_LEFT | CTextElement::T_VCENTER);
}

// COutputSlot

bool COutputSlot::ConnectToLine(CLine* line) {
	ATLASSERT(line);
	return line->SetOutputSlot(this);
}

void COutputSlot::CalcLinesOffset() {
	constexpr size_t max_n = 4;
	constexpr float dy = 2.6f;
	constexpr float max_y = -(float)(max_n / 2) * dy + ((float)max_n - 0.5f) * dy;
	constexpr float min_y = -(float)(max_n / 2) * dy + 0.5f * dy;

	size_t n = lines_.size();
	if (!n) return;

	if (n > 4)
		n = max_n;

	int x = 1;

	int even = n % 2 ? 0 : 1;
	float b = -(float)(n / 2) * dy;

	for (line_iterator it = lines_.begin(); it != lines_.end(); it++) {
		auto line = (*it);
		if (x > max_n) {
			if (line->GetP1().y < line->GetP2().y) line->SetLinePosition(dy * x, max_y);
			else line->SetLinePosition(dy * x, min_y);
		} else {
			auto y = (even ? b + ((float)x - 0.5f) * dy : b + ((float)x - 1.0f) * dy);
			line->SetLinePosition(dy * x, y);
		}
		line->Create();
		x++;
	}
}

void COutputSlot::AddLine(CLine* p_line) {
	m_bHasLines = TRUE;
	lines_.push_back(p_line);
}

void COutputSlot::DisconnectLine(CLine* pLine) {
	auto it = std::find(lines_.begin(), lines_.end(), pLine);
	ASSERT(it != lines_.end());
	lines_.erase(it);
	CalcLinesOffset();
	if (lines_.empty()) m_bHasLines = FALSE;
}

void COutputSlot::SetValue(const nps::server_value* value) {
	CSlot::SetValue(value);
	if (lines_.empty()) return;

	CLine::LineWidth width = CLine::LineWidth::Normal;
	SolidColor color = SolidColor::Black;

	if (value == nullptr) {
		for (auto& l : lines_) l->SetWidthAndColor(width, color);
		return;
	}

	switch (value->s)
	{
	case nps::var_status::VST_DEVICE_NOT_RESPONDED:
		color = SolidColor::LineDeviceNotResponded;
		break;
	case nps::var_status::VST_DEVICE_RESPONDED:
		try {
			bool has_quality = !npsys::variable::IsIO(value_.get_type()) && npsys::variable::IsQuality(value_.get_type());
			if (value_.is_quality_not_good()) {
				color = SolidColor::LineBadQuality;
				color = SolidColor::LineBadQuality;
			} else {
				if (value_.is_discrete()) {
					if (has_quality) {
						const auto& qbit = value_.to_Qbit();
						if (qbit.value == NPSYSTEM_TRUE) width = CLine::LineWidth::Bold;
					} else {
						const auto& bit = value_.to_bit();
						if (bit.value == NPSYSTEM_TRUE) width = CLine::LineWidth::Bold;
					}
				}
				color = SolidColor::Black;
			}
		} catch (std::bad_cast& ex) {
			if (value_.is_discrete()) {
				const bit& bit = value_.to_bit();
				if (bit.value) width = CLine::LineWidth::Bold;
			}
			color = SolidColor::Black;
			std::cerr << ex.what() << '\n';
		} catch (std::exception& ex) {
			color = SolidColor::Black;
			std::cerr << ex.what() << '\n';

		}
		break;
	default:
		color = SolidColor::LightGray;
		break;
	}
	for (auto& l : lines_) l->SetWidthAndColor(width, color);
}

void COutputSlot::AlignOnlineValue() {
	DWRITE_TEXT_METRICS metrics;
	Online.text_layout->GetMetrics(&metrics);
	Online.rc.rect.left = metrics.left + rect_.left + 25;
	Online.rc.rect.top = metrics.top + rect_.top - 20;
	Online.rc.rect.right = Online.rc.rect.left + metrics.width;
	Online.rc.rect.bottom = Online.rc.rect.top + metrics.height;
	Online.rc.radiusX = 1;
	Online.rc.radiusY = 1;
}

void COutputSlot::AlignText() {
	D2D1::MyPoint2F pt = GetMiddle();
	text_.SetPosition(pt.x - 10, pt.y - 0.5f, CTextElement::T_RIGHT | CTextElement::T_VCENTER);
}

// CSlotGroup
CSlotGroup::CSlotGroup(CSlot* slot) {
	//	elements_[0] = slot;
	//	CStaticTextElement* text = new CStaticTextElement;
	//	text->SetText(slot->GetName());
	//	slot->AlignText(text);
	//	elements_[1] = text;
}
CSlotGroup::~CSlotGroup() {
	delete GetText();
}
CElement::_Iterator* CSlotGroup::CreateIterator() {
	iterator_.SetRange(elements_.begin(), elements_.end());
	return static_cast<CElement::_Iterator*>(&iterator_);
}
void CSlotGroup::Move(float x, float y) {
	float dx = x - rect_.left;
	float dy = y - rect_.top;
	CGeometryElement::Move(x, y);
	for (auto i : elements_) {
		i->MoveDxDy(dx, dy);
	}
}
void CSlotGroup::MoveDxDy(float dx, float dy) {
	CGeometryElement::MoveDxDy(dx, dy);
	for (auto i : elements_)
		i->MoveDxDy(dx, dy);
}
void CSlotGroup::CreateRgnShape() noexcept {
	dis::Get().d2dFactory->CreateRectangleGeometry(rect_,
		(ID2D1RectangleGeometry**)m_geometry.GetAddressOf());
}

// CBlockInput

void CBlockInput::ReleaseMemory() noexcept {
}

npsys::variable_n* CBlockInput::GetVariableAsNode() {
	return nullptr;
}



// CSlotType
void CSlotType::PrintDebugTooltip(std::stringstream& ss) {
	auto var = GetVariable();
	if (var) {
		ss << *var << "\n";
	}
}

// CValue

CValue::CValue(int type) {
	variable_ = std::move(odb::create_node<npsys::variable_n>(type));
}

int CValue::DefineTypeR() {
	ASSERT_FETCH(variable_);
	return variable_->GetType();
}

npsys::variable_n* CValue::GetVariableAsNode() {
	return &variable_;
};

bool CValue::Equal(CValue* p) {
	return npsys::variable::TypesEqual(variable_->GetType(), p->variable_->GetType());
}

void CValue::ReleaseMemory() noexcept {
	COneVariable::ReleaseMemory();
}

void CValue::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	color[slot->e_slot.get()] = 2;
	sorted.push_front({ alg, slot, this, SlotType::Value });
}

void CFixedValue::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	color[slot->e_slot.get()] = 2;
	sorted.push_front({ alg, slot, this, SlotType::Value });
}

void CFixedValue::Hardware_InitVariable(odb::weak_node<npsys::device_n> dev, int type, uint16_t addr) {
	ASSERT(type & npsys::variable::INTERNAL);
	variable_ = odb::create_node<npsys::variable_n>(type);
	variable_->SetDev(dev);
	variable_->SetAddr(addr);
	variable_->AddRef();
	variable_->SetStatus(npsys::variable::Status::good);
	variable_.store();
}

void CValue::FillPropertyGrid(CPropertyGrid* grid) noexcept {
	using vt = npsys::variable::Type;
	npsys::CMyVariant var;

	switch (variable_->GetClearType())
	{
	case vt::VT_DISCRETE:
		var = variable_->DefaultValue_GetValue().d;
		break;
	case vt::VT_BYTE:
		var = variable_->DefaultValue_GetValue().u8;
		break;
	case vt::VT_SIGNED_BYTE:
		var = variable_->DefaultValue_GetValue().i8;
		break;
	case vt::VT_WORD:
		var = variable_->DefaultValue_GetValue().u16;
		break;
	case vt::VT_SIGNED_WORD:
		var = variable_->DefaultValue_GetValue().i16;
		break;
	case vt::VT_DWORD:
		var = variable_->DefaultValue_GetValue().u32;
		break;
	case vt::VT_SIGNED_DWORD:
		var = variable_->DefaultValue_GetValue().i32;
		break;
	case vt::VT_FLOAT:
		var = variable_->DefaultValue_GetValue().flt;
		break;
	default:
		return;
	}

	grid->AddSection(PRB::ID_SECTION_BLOCK_SPECIAL, "Special");
	grid->AddVariantItem(PRB::ID_SECTION_BLOCK_SPECIAL,
		"Default value", var, PRB::ID_PR_DEFAULT_VALUE);
}

void CValue::OnPropertyChanged(WPARAM wParam, LPARAM lParam) {
	ASSERT(lParam == PRB::ID_PR_DEFAULT_VALUE);

	const auto& v = *(CPropertyGrid::Variant*)(wParam);
	const auto& mv = boost::get<npsys::CMyVariant>(v);

	using TP = npsys::variable::Type;
	switch (variable_->GetClearType())
	{
	case TP::VT_DISCRETE:
		variable_->DefaultValue_SetValue(mv.as<bool>());
		break;
	case TP::VT_BYTE:
		variable_->DefaultValue_SetValue(mv.as<uint8_t>());
		break;
	case TP::VT_SIGNED_BYTE:
		variable_->DefaultValue_SetValue(mv.as<int8_t>());
		break;
	case TP::VT_WORD:
		variable_->DefaultValue_SetValue(mv.as<uint16_t>());
		break;
	case TP::VT_SIGNED_WORD:
		variable_->DefaultValue_SetValue(mv.as<int16_t>());
		break;
	case TP::VT_DWORD:
		variable_->DefaultValue_SetValue(mv.as<uint32_t>());
		break;
	case TP::VT_SIGNED_DWORD:
		variable_->DefaultValue_SetValue(mv.as<int32_t>());
		break;
	case TP::VT_FLOAT:
		variable_->DefaultValue_SetValue(mv.as<float>());
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

// CInternalRef
CInternalRef::CInternalRef() {
	link_name_ = _invalid_link_;
}

CInternalRef::~CInternalRef() {
	con_.disconnect();
}

int CInternalRef::DefineTypeR() {
	if (!link_valid_) return npsys::variable::VT_UNDEFINE;
	return const_cast<CSlot*>(slot_link_)->DefineTypeR();
}

npsys::variable_n* CInternalRef::GetVariableAsNode() {
	return link_valid_
		? const_cast<CSlot*>(slot_link_)->GetVariableAsNode()
		: nullptr;
};

void CInternalRef::OnRefSlotStateChanged(SLOT_SIGNAL s) noexcept {
	switch (s)
	{
	case SIG_SLOT_DELETED:
		ASSERT(slot_link_);

		saved_slot_link_ = slot_link_;

		slot_id_ = INVALID_LINK_ID;
		slot_link_ = nullptr;
		link_name_ = _invalid_link_;
		link_valid_ = false;

		break;
	case SIG_SLOT_DELETED_PERMANENT:
		ASSERT(slot_link_ == nullptr);
		break;

	case SIG_SLOT_INSERTED:
		ASSERT(saved_slot_link_);

		slot_id_ = saved_slot_link_->GetId();
		slot_link_ = saved_slot_link_;
		link_name_ = saved_slot_link_->GetFullName();
		link_valid_ = false;

		saved_slot_link_ = nullptr;

		break;
	case SIG_SLOT_PARENT_NAME_CHANGED:
		if (link_name_dirty_) *link_name_dirty_ = true;
		link_name_ = slot_link_->GetFullName();
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

// calling only once from constructor or when references are being restored
void CInternalRef::SetInternalLink(CSlot* slot_link) noexcept {
	ATLASSUME(slot_link_ == nullptr);
	ATLASSUME(saved_slot_link_ == nullptr);

	slot_link_ = slot_link;

	if (slot_link) {
		slot_id_ = slot_link->GetId();
		link_name_ = slot_link->GetFullName();
		link_valid_ = true;
		con_ = slot_link->AdviceToEvents(
			std::bind(&CInternalRef::OnRefSlotStateChanged, this, std::placeholders::_1)
		);
	} else {
		slot_id_ = INVALID_LINK_ID;
		link_name_ = _invalid_link_;
		link_valid_ = false;
	}
}

void CInternalRef::ReleaseMemory() noexcept {
	// internal refs don't increase ref counter
}

bool CInternalRef::CheckCycle(color_m& color, void* slot_ptr) {
	if (!IsLinkValid()) {
		color[slot_ptr] = 2;
		return false;
	}

	color[slot_ptr] = 1;

	if (color[slot_link_] == 0 &&
		slot_link_->GetSlotType()->CheckCycle(color, slot_ptr)) {
		return true;
	} else if (color[slot_ptr] == 1) {
		return true;
	}

	color[slot_ptr] = 2;

	return false;
}

void CInternalRef::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	color[slot->e_slot.get()] = 1;

	if (IsLinkValid() && (color[slot_link_] == 0)) {
		auto slot = slot_link_->top->self_node.fetch();
		TopoSort(alg, slot, color, sorted);
	}

	color[slot->e_slot.get()] = 2;
	sorted.push_front({ alg, slot, this, SlotType::InternalReference });
}

// CAvrInternalPin

npsys::variable_n* CAvrInternalPin::GetVariableAsNode() {
	if (!link_valid_) return nullptr;
	if (IsLoaded()) return &pin_->GetVariable();
	return nullptr;
};

int CAvrInternalPin::DefineTypeR() {
	if (!link_valid_) return npsys::variable::VT_UNDEFINE;
	if ((pin_->GetStatus() == io::Status::relevant) && IsPinTypeEqualParamDirection()) {
		return pin_->GetVariableType();
	} else {
		return  npsys::variable::VT_UNDEFINE;
	}
}

void CAvrInternalPin::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	color[slot->e_slot.get()] = 2;
	sorted.push_front({ alg, slot , this, SlotType::Peripheral });
}

bool CAvrInternalPin::IsPinTypeEqualParamDirection() const noexcept {
	if (!link_valid_) return false;
	if (pin_->GetPinType() == io::INPUT &&
		dir_ == npsys::remote::ExtLinkType::Read)
		return true;
	if (pin_->GetPinType() == io::OUTPUT &&
		dir_ == npsys::remote::ExtLinkType::Write)
		return true;
	return false;
}

bool CAvrInternalPin::LoadLinkImpl() noexcept {
	do {
		if (!pin_.fetch()) break;
		auto ctrl = pin_->ctrl.fetch();
		if (!ctrl.loaded()) break;
		auto port = pin_->port.fetch();
		if (!port.loaded()) break;
		link_name_ = '/' + ctrl->get_name() + "/IO/" + port->get_name() + '/' + pin_->get_name();
		return link_valid_ = true;
	} while (0);

	link_name_ = _invalid_peripheral_link_;
	return link_valid_ = false;
}

void CAvrInternalPin::ReleaseMemory() noexcept {
	if (loaded_ && pin_.fetch()) {
		auto var = pin_->GetVariable();
		ASSERT_FETCH(var);

		ASSERT(!alg_.is_invalid_id());
		auto alg = alg_.fetch();

		ASSERT(!alg->assigned_alg.is_invalid_id());
		auto assigned = alg->assigned_alg.fetch();

		auto avr_assigned = static_cast<npsys::CAVRAssignedAlgorithm*>(
			assigned.get());

		avr_assigned->AddVariableReferenceToRemoveList(var);
		assigned.store();
		loaded_ = false;
	}
}

ParameterTrait CAvrInternalPin::GetParameterTraits() {
	ASSERT(link_valid_ && loaded_);
	return pin_->IsInverted() ? ParameterTrait(ParameterTrait::PT_INVERTED) : ParameterTrait();
}

// CModuleValue

CModuleValue::CModuleValue(npsys::i2c_assigned_module_n& mod,
	oid_t segment_value_id,
	npsys::remote::ExtLinkType link_type)
	: mod_(mod)
	, segment_value_id_(segment_value_id)
	, link_type_(link_type)
{
	LoadLink();
}

npsys::variable_n* CModuleValue::GetVariableAsNode() {
	if (link_valid_ && last_loaded_var_.fetch()) {
		if (!last_loaded_var_->IsGood()) return nullptr;
		return &last_loaded_var_;
	} else {
		return nullptr;
	}
};

int CModuleValue::DefineTypeR() {
	if (link_valid_
		&& last_loaded_var_.fetch()
		&& last_loaded_var_->GetStatus() != npsys::variable::Status::to_remove) {
		return last_loaded_var_->GetType();
	} else {
		return npsys::variable::VT_UNDEFINE;
	}
}

void CModuleValue::ReleaseMemory() noexcept {
	if (!last_loaded_var_.is_invalid_node()) {
		ASSERT_FETCH(last_loaded_var_);

		ASSERT(!alg_.is_invalid_id());
		auto alg = alg_.fetch();

		ASSERT(!alg->assigned_alg.is_invalid_id());
		auto assigned = alg->assigned_alg.fetch();

		assigned->AddVariableReferenceToRemoveList(last_loaded_var_);
		assigned.store();
		last_loaded_var_ = {};
	}
}

void CModuleValue::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	color[slot->e_slot.get()] = 2;
	sorted.push_front({ alg, slot, this, SlotType::ModuleValue });
}

bool CModuleValue::IsPinTypeEqualParamDirection() {
	if (!link_valid_) return false;
	auto val = mod_->GetSegmentValue(segment_value_id_);
	if (!val) return false;
	auto segment_type = val->parent->GetType();
	if (
		(segment_type == io::SegmentType::READ && link_type_ == npsys::remote::ExtLinkType::Read) ||
		(segment_type == io::SegmentType::WRITE && link_type_ == npsys::remote::ExtLinkType::Write)
		) {
		return true;
	} else {
		return false;
	}
}

void modify_link(int position, const std::string& item, std::string& link) {
	/*
	ASSERT(position > 0);

	int begin = 0, end = 0;

	do {
		end = link.Find(L'/', begin);
		if (end == -1) {
			end = link.GetLength();
			ASSERT(position == 0);
		}
		if (position != 0) begin = end + 1;
	} while (position--);

	ASSERT(begin > 0 && end > 0);

	if (begin == 1) {
		link = L'/' + item + link.Mid(end);
	} else if (end == link.GetLength()) {
		link = link.Mid(0, begin) + item;
	} else {
		link = link.Mid(0, begin) + item + link.Mid(end, link.GetLength() - end);
	}
	*/
}

bool CModuleValue::LoadLinkImpl() noexcept {
	do {
		if (!mod_.fetch()) break;
		value = mod_->GetSegmentValue(segment_value_id_);
		if (!value) break;
		
		auto ctrl = mod_->ctrl.fetch();
		if (!ctrl.loaded()) break;

		link_name_ =
			'/' + ctrl->get_name() +
			"/I2C/" + mod_->get_name() +
			'/' + value->parent->get_name() +
			'/' + value->get_name();

		con_ = value->ass->observe_remove([this]() {
			ATLASSUME(link_valid_);
			mod_ = {};
			segment_value_id_ = -1;
			link_name_ = _invalid_link_;
			link_valid_ = false;
		});
		
		con_ctrl_name_ = ctrl->observe_name([this](const std::string& name) { 
			modify_link(1, name, link_name_); 
		});

		con_module_name_ = mod_->observe_name([this](const std::string& name) { 
			modify_link(3, name, link_name_); 
		});

		con_module_segment_ = value->parent->observe_name([this](const std::string& name) { 
			modify_link(4, name, link_name_); 
		});

		con_module_segment_value_ = value->observe_name([this](const std::string& name) { 
			modify_link(5, name, link_name_); 
		});
		
		return link_valid_ = true;
	} while (0);

	link_name_ = _invalid_link_;
	return link_valid_ = false;
}

void CModuleValue::PrintDebugTooltip(std::stringstream& ss) {
	if (!last_loaded_var_.is_invalid_node()) {
		ASSERT_FETCH(last_loaded_var_);
		ss << *last_loaded_var_.get() << "\n";
	} else {
		ss << "llv null" << "\n";
	}
}
// CExternalReference

CExternalReference::CExternalReference(
	npsys::fbd_slot_n& ref_slot,
	npsys::algorithm_n& ref_alg,
	npsys::algorithm_n& alg)
	: COutsideReference(alg)
	, loaded_(false) {
	link_ = LinkData(ref_slot, ref_alg);
	LoadLink();
	auto ref_var = ref_slot->e_slot->GetVariable();
	variable_ = std::move(
		odb::create_node<npsys::variable_n>(ref_var->GetClearType())
	);
}

int CExternalReference::DefineTypeR() {
	auto ref_slot = GetRefSlot();
	return ref_slot ? ref_slot->DefineTypeR() : npsys::variable::VT_UNDEFINE;
}

npsys::variable_n* CExternalReference::GetVariableAsNode() {
	return &variable_;
}

void CExternalReference::ReleaseMemory() noexcept {
	auto alg = alg_.fetch();
	if (alg->assigned_alg.is_invalid_id()) return;

	if (loaded_) {
		ASSERT_FETCH(variable_);
		auto assigned = alg->assigned_alg.fetch();
		if (place_ == Place::SAME_DEVICE) {
			assigned->AddVariableReferenceToRemoveList(variable_);
		} else {
			ASSERT(!link_.loaded_variable.is_invalid_id());
			variable_->OwnerRelease(nullptr);
			assigned->AddExternalReferenceToRemoveList(this);
			link_.loaded_variable = {};
		}
		loaded_ = false;
		assigned.store();
	}
}

bool CExternalReference::Equal(CExternalReference* other) {
	return GetLinkName() == other->GetLinkName();
}

bool CExternalReference::CheckCycle(color_m& color, void* slot_ptr) {
	if (!link_valid_) {
		color[slot_ptr] = 2;
		return false;
	}

	color[slot_ptr] = 1;

	auto ref_slot = GetRefSlot();
	if (ref_slot) {
		if (color[ref_slot] == 0) {
			if (ref_slot->GetSlotType()->CheckCycle(color, ref_slot))
				return true;
		} else if (color[ref_slot] == 1) {
			return true;
		}
	}

	color[slot_ptr] = 2;
	return false;
}

void CExternalReference::TopoSort(
	npsys::algorithm_n& alg,
	npsys::fbd_slot_n& slot,
	color_m& color,
	SortedLink& sorted)
{
	ASSERT(alg.id() == alg_.id());

	if (!link_valid_) {
		color[slot->e_slot.get()] = 2;
		return;
	}

	color[slot->e_slot.get()] = 1;

	if (link_valid_) {
		if (color[link_.slot->e_slot.get()] == 0) {
			auto link_alg = link_.alg.fetch();
			link_.slot->e_slot->GetSlotType()->TopoSort(link_alg, link_.slot, color, sorted);
		} else if (color[link_.slot->e_slot.get()] == 1) {
			MessageBoxA(g_hMainWnd, "There is a cyckle", "Error", MB_ICONERROR);
			ASSERT(FALSE);
		}
	}

	sorted.push_front({ alg, slot, this, SlotType::ExternalReference });
	color[slot->e_slot.get()] = 2;
}

CSlot* CExternalReference::GetRefSlot() noexcept {
	return link_valid_ ? link_.slot->e_slot.get() : nullptr;
}

npsys::variable_n* CExternalReference::GetRefVariableAsNode() noexcept {
	auto ref_slot = GetRefSlot();
	return ref_slot ? ref_slot->GetVariableAsNode() : nullptr;
}

bool CExternalReference::LoadLinkImpl() noexcept {
	do {
		if (!link_.slot.fetch()) break;
		if (link_.slot->e_slot->IsDeletedFromEditor()) {
			con_ = link_.slot->e_slot->AdviceToEvents(
				std::bind(&CExternalReference::OnRefSlotStateChanged, this, std::placeholders::_1)
			);
			break;
		}
		auto ref_alg = link_.alg.fetch();
		auto ref_cat = ref_alg->cat.fetch();
		link_name_ = '/' + ref_cat->get_name() + '/'
			+ ref_alg->get_name() + '/' + link_.slot->e_slot->GetFullName();
		con_ = link_.slot->e_slot->AdviceToEvents(
			std::bind(&CExternalReference::OnRefSlotStateChanged, this, std::placeholders::_1)
		);
		return link_valid_ = true;
	} while (0);

	link_name_ = _invalid_external_link_;
	return link_valid_ = false;
}

void CExternalReference::OnRefSlotStateChanged(SLOT_SIGNAL s) noexcept {
	switch (s)
	{
	case SIG_SLOT_DELETED:
		ATLASSUME(link_valid_);
		link_name_ = _invalid_external_link_;
		link_valid_ = false;
		break;
	case SIG_SLOT_DELETED_PERMANENT:
		ATLASSUME(!link_valid_);
		link_.slot = {};
		break;
	case SIG_SLOT_INSERTED:
		ATLASSUME(!link_valid_);
		link_valid_ = true;
		[[fallthrough]];
	case SIG_SLOT_PARENT_NAME_CHANGED:
	{
		ATLASSUME(link_valid_);
		if (link_name_dirty_) *link_name_dirty_ = true;
		auto ref_alg = link_.alg.fetch();
		auto ref_cat = ref_alg->cat.fetch();
		link_name_ = '/' + ref_cat->get_name() + '/'
			+ ref_alg->get_name() + '/' + link_.slot->e_slot->GetFullName();

		break;
	}
	default:
		ASSERT(FALSE);
		break;
	}
}

bool CExternalReference::IsRefAlgorithmAssigned() noexcept {
	if (link_.alg.is_invalid_id()) return false;
	auto ref_alg = link_.alg.fetch();
	if (!ref_alg.loaded()) return false;
	return (ref_alg->GetStatus() >= npsys::CAlgorithm::status_assigned);
}

void CExternalReference::PrintDebugTooltip(std::stringstream& ss) {
	__super::PrintDebugTooltip(ss);
	auto var = GetRefVariableAsNode();
	if (var) {
		ss << *(*var).get() << "\n";
	}
	if (!link_.loaded_variable.is_invalid_id()) {
		auto var = link_.loaded_variable.fetch();
		if (var.loaded()) {
			ss << *var.get() << "\n";
		}
	}
}

// CSlotManipulator

S_CURSOR CSlot::CSlotManipulator::MouseButtonDown(CPoint point, CAlgorithmView* pWnd) {
	line_.reset(new CLine);

	auto middle = _slot->GetMiddle();
	begin_at_ = _slot->GetDirection();

	line_->SetP1(middle);
	line_->SetP2(middle);

	first_slot_no_error_ = line_->SetSlot(_slot);

	auto parent = _slot->GetParentBlock();

	fs.clear();
	auto blocks = pWnd->alg_->GetBlocks();
	auto begin = parent->top->slots.begin();
	auto end = parent->top->slots.end();
	Iter::PostorderIterator<CElement*> it(blocks);
	for (; !it.IsDone(); it.Next()) {
		auto slot = *it;
		if (slot == _slot) continue;
		if (std::find_if(begin, end,
			[slot](auto& slot_n) {
			return slot == slot_n->e_slot.get();
		}) != end) {
			continue;
		}
		(*it)->Visit(fs);
	}
	return S_CURSOR::A_LINE;
}

void CSlot::CSlotManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	CGraphics* pGraphics = pWnd->GetGraphics();
	const D2D1_MATRIX_3X2_F& invM = pGraphics->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pGraphics->GetMatrix();

	D2D1::MyPoint2F pt = D2D1::MyPoint2F((float)point.x, (float)point.y) * invM;

	if (begin_at_ == CSlot::INPUT_SLOT) line_->SetP1(pt);
	else line_->SetP2(pt);

	for (auto slot : fs) {
		UINT elflags;
		if (slot->HitTest(pt, elflags)) {
			_tmp_slot = slot;
			break;
		}
	}
	pWnd->Invalidate();
}

void CSlot::CSlotManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	CGraphics* pGraphics = pWnd->GetGraphics();
	const D2D1_MATRIX_3X2_F& invM = pGraphics->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pGraphics->GetMatrix();

	_slot->SetHover(false);

	D2D1::MyPoint2F pt = D2D1::MyPoint2F((float)point.x, (float)point.y) * invM;
	for (auto& i : fs) {
		UINT elflags;
		if (i->HitTest(pt, elflags)) {
			if (first_slot_no_error_ && line_->SetSlot(i)) {
				pWnd->SetCursor(AlphaCursor::Arrow);
				pWnd->Insert(line_.release());
				return;
			}
			MessageBoxA(pWnd->m_hWnd, "Wrong operation...", "Error", 0);
			break;
		}
	}

	line_.reset();

	pWnd->SetCursor(AlphaCursor::Arrow);
	pWnd->Invalidate();
}

void CSlot::CSlotManipulator::Draw(CGraphics* pGraphics) {
	line_->PreDraw(pGraphics);
}