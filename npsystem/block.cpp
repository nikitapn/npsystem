// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "memorymanager.h"
#include "graphics.h"
#include "block.h"
#include "line.h"
#include "codegenerator.h"
#include "command.h"
#include "blockcomposition.h"
#include "dlgonlinediscrete.h"
#include "dlgonlinevalue.h"
#include "dlgblockparam.h"
#include "blockpropertyid.h"
#include "view_algorithm.h"
#include "graphics.h"
#include <npsys/fbdblock.h>
#include <npsys/guid.h>
#include "network_ext.h"
#include "genhelper.h"

#define IMPLEMENT_TRANSLATE(class_name) \
void class_name ## ::Translate(CCodeGenerator* pGenerator) { \
	pGenerator->Generate(this); \
}


const std::string CElement::emptyString_;

// Special:CInput
NP_BOOST_CLASS_EXPORT_GUID(CInput);
IMPLEMENT_VISITOR(CInput);
IMPLEMENT_TRANSLATE(CInput);
// Special:COutput
NP_BOOST_CLASS_EXPORT_GUID(COutput);
IMPLEMENT_VISITOR(COutput);
IMPLEMENT_TRANSLATE(COutput);
// Logic:COr
NP_BOOST_CLASS_EXPORT_GUID(COr);
IMPLEMENT_VISITOR(COr);
IMPLEMENT_TRANSLATE(COr);
// Logic:CAnd
NP_BOOST_CLASS_EXPORT_GUID(CAnd);
IMPLEMENT_VISITOR(CAnd);
IMPLEMENT_TRANSLATE(CAnd);
// Logic:CNot
NP_BOOST_CLASS_EXPORT_GUID(CNot);
IMPLEMENT_VISITOR(CNot);
IMPLEMENT_TRANSLATE(CNot);
// Logic:CPositiveEdge
NP_BOOST_CLASS_EXPORT_GUID(CPositiveEdge);
IMPLEMENT_VISITOR(CPositiveEdge);
IMPLEMENT_TRANSLATE(CPositiveEdge);
// Logic:CNegativeEdge
NP_BOOST_CLASS_EXPORT_GUID(CNegativeEdge);
IMPLEMENT_VISITOR(CNegativeEdge);
IMPLEMENT_TRANSLATE(CNegativeEdge);
// Logic:CAnyEdge
NP_BOOST_CLASS_EXPORT_GUID(CAnyEdge);
IMPLEMENT_VISITOR(CAnyEdge);
IMPLEMENT_TRANSLATE(CAnyEdge);
// Logic:CRsTrigger
NP_BOOST_CLASS_EXPORT_GUID(CRsTrigger);
IMPLEMENT_VISITOR(CRsTrigger);
IMPLEMENT_TRANSLATE(CRsTrigger);
// Logic:CDelay
NP_BOOST_CLASS_EXPORT_GUID(CDelay);
IMPLEMENT_VISITOR(CDelay);
IMPLEMENT_TRANSLATE(CDelay);
// Logic:CBinaryEncoder
NP_BOOST_CLASS_EXPORT_GUID(CBinaryEncoder);
IMPLEMENT_VISITOR(CBinaryEncoder);
IMPLEMENT_TRANSLATE(CBinaryEncoder);
// Logic:CBinaryDecoder
NP_BOOST_CLASS_EXPORT_GUID(CBinaryDecoder);
IMPLEMENT_VISITOR(CBinaryDecoder);
IMPLEMENT_TRANSLATE(CBinaryDecoder);
// Logic:CCounter
NP_BOOST_CLASS_EXPORT_GUID(CCounter);
IMPLEMENT_VISITOR(CCounter);
IMPLEMENT_TRANSLATE(CCounter);
// Math:CAdd
NP_BOOST_CLASS_EXPORT_GUID(CAdd);
IMPLEMENT_VISITOR(CAdd);
IMPLEMENT_TRANSLATE(CAdd);
// Math:CSub
NP_BOOST_CLASS_EXPORT_GUID(CSub);
IMPLEMENT_VISITOR(CSub);
IMPLEMENT_TRANSLATE(CSub);
// Math:CMul
NP_BOOST_CLASS_EXPORT_GUID(CMul);
IMPLEMENT_VISITOR(CMul);
IMPLEMENT_TRANSLATE(CMul);
// Math:CDiv
NP_BOOST_CLASS_EXPORT_GUID(CDiv);
IMPLEMENT_VISITOR(CDiv);
IMPLEMENT_TRANSLATE(CDiv);
// Math:CComparator
NP_BOOST_CLASS_EXPORT_GUID(CComparator);
IMPLEMENT_VISITOR(CComparator);
IMPLEMENT_TRANSLATE(CComparator);
// Math:CBlockFunction
NP_BOOST_CLASS_EXPORT_GUID(CBlockFunction);
IMPLEMENT_VISITOR(CBlockFunction);
IMPLEMENT_TRANSLATE(CBlockFunction);
// PID
NP_BOOST_CLASS_EXPORT_GUID(CPID);
IMPLEMENT_VISITOR(CPID);
IMPLEMENT_TRANSLATE(CPID);
// CAlarmHigh
NP_BOOST_CLASS_EXPORT_GUID(CAlarmHigh);
IMPLEMENT_VISITOR(CAlarmHigh);
IMPLEMENT_TRANSLATE(CAlarmHigh);
// CAlarmLow
NP_BOOST_CLASS_EXPORT_GUID(CAlarmLow);
IMPLEMENT_VISITOR(CAlarmLow);
IMPLEMENT_TRANSLATE(CAlarmLow);
// CTime
NP_BOOST_CLASS_EXPORT_GUID(CTime);
IMPLEMENT_VISITOR(CTime);
IMPLEMENT_TRANSLATE(CTime);
// CBlockSchedule
NP_BOOST_CLASS_EXPORT_GUID(CBlockSchedule);
IMPLEMENT_VISITOR(CBlockSchedule);
IMPLEMENT_TRANSLATE(CBlockSchedule);
// CScheduleSlider
NP_BOOST_CLASS_EXPORT_GUID(CScheduleSlider);
IMPLEMENT_VISITOR(CScheduleSlider);
// CSliderTimeChart
NP_BOOST_CLASS_EXPORT_GUID(CSliderTimeChart);
IMPLEMENT_VISITOR(CSliderTimeChart);
// CSliderThing
NP_BOOST_CLASS_EXPORT_GUID(CSliderThing);
IMPLEMENT_VISITOR(CSliderThing);
// CTextElement
NP_BOOST_CLASS_EXPORT_GUID(CTextElement);
IMPLEMENT_VISITOR(CTextElement);
// CStaticTextElement
NP_BOOST_CLASS_EXPORT_GUID(CStaticTextElement);
IMPLEMENT_VISITOR(CStaticTextElement);
// CDxEdit
IMPLEMENT_VISITOR(CDxEdit);

BOOST_CLASS_EXPORT_GUID(BlockPropertyT<bool>, NPG_BlockPropBool);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<uint8_t>, NPG_BlockPropU8);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<int8_t>, NPG_BlockPropI8);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<uint16_t>, NPG_BlockPropU16);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<int16_t>, NPG_BlockPropI16);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<uint32_t>, NPG_BlockPropU32);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<int32_t>, NPG_BlockPropI32);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<float>, NPG_BlockPropFloat);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<double>, NPG_BlockPropDouble);
BOOST_CLASS_EXPORT_GUID(BlockPropertyT<std::string>, NPG_BlockPropCString);

int CBlockFactory::SetNewId(CSlot* slot) {
	auto old_id = slot->id_;
	slot->id_ = next_id();
	return old_id;
}

npsys::fbd_block_n CBlockFactory::CreateBlock(WORD wID) {
	npsys::fbd_block_n n;
	switch (wID) {
	case ID_CREATE_BLOCK_INPUT:
		CreateInput(n);
		break;
	case ID_CREATE_BLOCK_OUTPUT:
		CreateOutput(n);
		break;
	case ID_CREATE_BLOCK_OR:
		CreateOr(n);
		break;
	case ID_CREATE_BLOCK_AND:
		CreateAnd(n);
		break;
	case ID_CREATE_BLOCK_NOT:
		CreateNot(n);
		break;
	case ID_CREATE_BLOCK_RS:
		CreateRsTrigger(n);
		break;
	case ID_CREATE_BLOCK_PE:
		CreatePositiveEdge(n);
		break;
	case ID_CREATE_BLOCK_NE:
		CreateNegativeEdge(n);
		break;
	case ID_CREATE_BLOCK_AE:
		CreateAnyEdge(n);
		break;
	case ID_CREATE_BLOCK_BINARY_ENCODER:
		CreateBinaryEncoder(n);
		break;
	case ID_CREATE_BLOCK_BINARY_DECODER:
		CreateBinaryDecoder(n);
		break;
	case ID_CREATE_BLOCK_DELAY:
		CreateDelay(n);
		break;
	case ID_CREATE_BLOCK_ADD:
		CreateAdd(n);
		break;
	case ID_CREATE_BLOCK_SUB:
		CreateSub(n);
		break;
	case ID_CREATE_BLOCK_MUL:
		CreateMul(n);
		break;
	case ID_CREATE_BLOCK_DIV:
		CreateDiv(n);
		break;
	case ID_CREATE_BLOCK_COMPARATOR:
		CreateComparator(n);
		break;
	case ID_CREATE_BLOCK_FUNCTION:
		CreateFunction(n);
		break;
	case ID_CREATE_BLOCK_PID:
		CreatePID(n);
		break;
	case ID_CREATE_BLOCK_CMP_ABOVE:
		CreateAlarmHigh(n);
		break;
	case ID_CREATE_BLOCK_CMP_BELOW:
		CreateAlarmLow(n);
		break;
	case ID_CREATE_BLOCK_TIME:
		CreateTime(n);
		break;
	case ID_CREATE_BLOCK_SCHEDULE:
		CreateSchedule(n);
		break;
	case ID_CREATE_BLOCK_COUNTER:
		CreateCounter(n);
		break;
	default:
		ATLASSERT(FALSE);
		return {};
	}
	return n;
}

void
CBlockFactory::CreateOutput(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new COutput;
	n->e_block.reset(block);
	block->top = n.get();
//	block->st_new_block_ = true;

	const std::string pattern = block->GetTypeNameStr();

	if (pattern.length()) {
		if (blocks_) {
			auto name = blocks_->CreateUniqueName(pattern);
			block->top->set_name(name);
		}
	}

	auto text_name = new CTextElement(std::string(block->GetName()),
		constants::block::PARAM_WIDTH - constants::block::SLOT_SPACE, 
		constants::block::PARAM_HEIGHT / 2.0f - 0.5f, 
		CTextElement::T_RIGHT | CTextElement::T_VCENTER);

	block->elements_.insert(block->elements_.begin(), text_name);
	block->MountSlot(CreateInputSlot("IN", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->SetParamType(PARAMETER_TYPE::P_VALUE);
	block->AdjustBlock();
}

void
CBlockFactory::CreateInput(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();

	auto block = new CInput;
	n->e_block.reset(block);
	block->top = n.get();
//block->st_new_block_ = true;

	const std::string pattern = block->GetTypeNameStr();
	if (pattern.length()) {
			if (blocks_) {
				auto name = blocks_->CreateUniqueName(pattern);
				block->top->set_name(name);
		}
	}

	auto text_name = new CTextElement(std::string(block->GetName()),
		constants::block::SLOT_SPACE, 
		constants::block::PARAM_HEIGHT / 2.0f - 0.5f, 
		CTextElement::T_LEFT | CTextElement::T_VCENTER);

	block->elements_.insert(block->elements_.begin(), text_name);
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->SetParamType(PARAMETER_TYPE::P_VALUE);
	block->AdjustBlock();
}

npsys::fbd_slot_n CBlockFactory::CreateSlot(const std::string& name, CSlot* slot, npsys::fbd_block_n& block) {
	slot->top = new npsys::CFBDSlot(name, block);
	slot->top->e_slot.reset(slot);
	slot->text_.SetText(slot->GetName());
	slot->AlignText();

	npsys::fbd_slot_n n;
	n.create_locally_with_new_id(slot->top);
	return n;
}

npsys::fbd_slot_n CBlockFactory::CreateInputSlot(const std::string& name, npsys::fbd_block_n& block, CSlotType* slot_type) {
	return CreateSlot(name, new CInputSlot(next_id(), slot_type), block);
}

npsys::fbd_slot_n CBlockFactory::CreateOutputSlot(const std::string& name, npsys::fbd_block_n& block, CSlotType* slot_type) {
	return CreateSlot(name, new COutputSlot(next_id(), slot_type), block);
}

void CBlock::MountSlot(npsys::fbd_slot_n&& slot) {
	top->set_modified();
	slot->set_modified();

	auto e_slot = slot->e_slot.get();
	e_slot->SetParent(this);
	elements_.push_back(e_slot);
	if (e_slot->GetDirection() == CSlot::INPUT_SLOT) {
		e_slot->SetSlotPosition(top->slots.i_size(), rect_.left, rect_.top, rect_.right - rect_.left);
		top->slots.add_input_slot(slot);
	} else {
		e_slot->SetSlotPosition(top->slots.o_size(), rect_.left, rect_.top, rect_.right - rect_.left);
		top->slots.add_output_slot(slot);
	}
}

void
CBlockFactory::CreateBlock(CBlock* block) {
	const std::string pattern = block->GetTypeNameStr();
	if (pattern.length()) {
		if (blocks_) {
			auto name = blocks_->CreateUniqueName(pattern);
			block->top->set_name(name);
		}
	}

	auto block_width = block->rect_.right - block->rect_.left;

	{
		auto text_name = new CTextElement(std::string(block->GetName()),
			block_width / 2, -10.0f, CTextElement::T_CENTER | CTextElement::T_VCENTER);
		text_name->ResetHitAccess();
		text_name->ShowElement(false);
		block->elements_.insert(block->elements_.begin(), text_name);
	}
	{
		auto type_name = new CTextElement(std::string(block->GetTypeNameStr()),
			block_width / 2, constants::block::HEAD_HEIGHT / 2, CTextElement::T_CENTER | CTextElement::T_VCENTER);
		type_name->ResetHitAccess();
		block->elements_.push_back(type_name);
	}
	block->CreateRgnShape();
}

void
CBlockFactory::CreateOr(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new COr;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
}

void
CBlockFactory::CreateAnd(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CAnd;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
}

void
CBlockFactory::CreateRsTrigger(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CRsTrigger;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("SET",  n, new CBlockInput));
	block->MountSlot(CreateInputSlot("RESET", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
}

void
CBlockFactory::CreateNot(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CNot;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(1);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN",  n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
}

void
CBlockFactory::CreatePositiveEdge(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CPositiveEdge;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(1);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->variable_ = odb::create_node<npsys::variable_n>(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY);
}

void
CBlockFactory::CreateNegativeEdge(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CNegativeEdge;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(1);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->variable_ = odb::create_node<npsys::variable_n>(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY);
}

void
CBlockFactory::CreateAnyEdge(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CAnyEdge; 
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(1);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->variable_ = odb::create_node<npsys::variable_n>(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY);
}

void
CBlockFactory::CreateDelay(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CDelay;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->variable_ = odb::create_node<npsys::variable_n>(npsys::variable::VT_DWORD | npsys::variable::VQUALITY);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT",n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
//	block->AddGraphicElement(new CBlockVariantProperty(D2D1::RectF(10, Y_POSITION(1), 
//		constants::block::BLOCK_WIDTH - 10, Y_POSITION(2)), "Delay(ms):", &block->timeout_));
	block->AddProperty<int>(PRB::Section::ID_SECTION_BLOCK_SPECIAL, "Timeout", "timeout", 1000);
	block->CreateInplaceProperties();
}

void
CBlockFactory::CreateBinaryEncoder(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CBinaryEncoder;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(8);
	CreateBlock(block);
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	for (int i = 0; i < 8; i++) {
		block->MountSlot(CreateInputSlot("IN_" + std::to_string(i + 1), n, new CBlockInput));
	}
}

void
CBlockFactory::CreateBinaryDecoder(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CBinaryDecoder;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(16);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	for (int i = 0; i < 16; i++) {
		block->MountSlot(CreateOutputSlot("OUT_" + std::to_string(i + 1), n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	}
}

void
CBlockFactory::CreateAdd(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CAdd;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_UNDEFINE | npsys::variable::MUTABLE)));
}

void
CBlockFactory::CreateSub(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CSub;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_UNDEFINE | npsys::variable::MUTABLE)));
}

void
CBlockFactory::CreateMul(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CMul;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_UNDEFINE | npsys::variable::MUTABLE)));
}

void
CBlockFactory::CreateDiv(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CDiv;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_UNDEFINE | npsys::variable::MUTABLE)));
}

void
CBlockFactory::CreateComparator(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CComparator;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	// -eq равно if [ "$a" -eq "$b" ]
	// -ne не равно if [ "$a" -ne "$b" ]
	// -gt больше if [ "$a" -gt "$b" ]
	// -ge больше или равно if [ "$a" -ge "$b" ]
	// -lt меньше if [ "$a" -lt "$b" ] 
	// -le меньше или равно if [ "$a" -le "$b" ]
	block->MountSlot(CreateInputSlot("IN_1", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("IN_2", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("EQ", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddDisabledSlot(CreateOutputSlot("NE", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddDisabledSlot(CreateOutputSlot("GT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddDisabledSlot(CreateOutputSlot("GE", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddDisabledSlot(CreateOutputSlot("LT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddDisabledSlot(CreateOutputSlot("LE", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
}

void
CBlockFactory::CreateFunction(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CBlockFunction;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(1);
	CreateBlock(block);

	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_FLOAT | npsys::variable::VQUALITY)));
}


void
CBlockFactory::CreatePID(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CPID;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(5);
	CreateBlock(block);

	block->variables_.push_back(odb::create_node<npsys::variable_n>(npsys::variable::VT_FLOAT | npsys::variable::VQUALITY));

	for (int i = 0; i < 6; ++i)
		block->variables_.push_back(odb::create_node<npsys::variable_n>(npsys::variable::VT_FLOAT));

	block->variables_[CPID::PID_KP]->DefaultValue_SetValue(1.0f);
	block->variables_[CPID::PID_KI]->DefaultValue_SetValue(0.0001f);
	block->variables_[CPID::PID_KD]->DefaultValue_SetValue(80.0f);

	block->MountSlot(CreateInputSlot("PV", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("SP", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValueRef(block->variables_[CPID::PID_OUT])));
}

void
CBlockFactory::CreateAlarmHigh(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CAlarmHigh;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("H", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddProperty<float>(PRB::Section::ID_SECTION_BLOCK_SPECIAL, "Hysteresis", "hyst", 10.0f);
}

void
CBlockFactory::CreateAlarmLow(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CAlarmLow;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("L", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->AddProperty<float>(PRB::Section::ID_SECTION_BLOCK_SPECIAL, "Hysteresis", "hyst", 10.0f);
}

void
CBlockFactory::CreateTime(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CTime;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(7);
	CreateBlock(block);
	block->MountSlot(CreateOutputSlot("SEC", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("MIN", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("HOUR", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("WDAY", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("MDAY", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("MONTH", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("YEAR", n, new CFixedValue));
}

void 
CBlockFactory::CreateSchedule(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CBlockSchedule;
	n->e_block.reset(block);
	block->top = n.get();
	block->rect_.right = constants::block::SCHEDULE_BLOCK_WIDTH;
	block->rect_.bottom = constants::block::SCHEDULE_BLOCK_HEIGHT;
	CreateBlock(block);
	
	auto slider = CreateScheduleSlider(block);
	block->AddGraphicElement(slider);
	slider->AddGraphicElement(CreateSliderTimeChart(slider));
	slider->AddGraphicElement(CreateSliderThing(slider));

	block->MountSlot(CreateInputSlot("ENABLED", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("OUT", n, new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY)));
	block->MountSlot(CreateOutputSlot("HOUR", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("MIN", n, new CFixedValue));
	block->MountSlot(CreateOutputSlot("SEC", n, new CFixedValue));

	block->AddProperty<int>(PRB::Section::ID_SECTION_BLOCK_SPECIAL, "Active time", "active_time", 1);
}

CScheduleSlider* 
CBlockFactory::CreateScheduleSlider(CBlockSchedule* parent) {
	auto pos = parent->GetPosition();
	auto element = new CScheduleSlider;
	element->CreateRgnShape();
	element->Move(pos.x + constants::slider::SCHEDULE_BLOCK_X_OFFSET, pos.y + constants::slider::SCHEDULE_BLOCK_Y_OFFSET);

	return element;
}

CSliderTimeChart* CBlockFactory::CreateSliderTimeChart(CScheduleSlider* parent) {
	auto pos = parent->GetPosition();
	auto element = new CSliderTimeChart;
	auto x = pos.x + constants::slider::SCHEDULE_BLOCK_TIME_CHART_OFFSET;
	
	element->CreateRgnShape();
	element->Move(x, pos.y + 13);

	auto const y = pos.y + 13 + 22;

	using namespace std::string_view_literals;

	static auto time_points = { "0"sv, "1"sv, "2"sv, "3"sv, "4"sv, "5"sv, "6"sv,
		"7"sv, "8"sv, "9"sv, "10"sv, "11"sv, "12"sv, "13"sv,
		"14"sv, "15"sv, "16"sv, "17"sv, "18"sv, "19"sv, "20"sv,
		"21"sv, "22"sv, "23"sv };

	for (auto tp : time_points) {
		auto text = new CTextElement(std::string(tp), x, y, CTextElement::T_CENTER | CTextElement::T_VCENTER, CTextElement::FONT_SIZE_8);
		element->AddGraphicElement(text);
		x += constants::slider::SLIDER_STEP * 4;
	}

	return element;
}

CSliderThing* 
CBlockFactory::CreateSliderThing(CScheduleSlider* parent) {
	auto pos = parent->GetPosition();
	auto element = new CSliderThing(parent);
	element->CreateRgnShape();
	element->Move(pos.x + constants::slider::thing::offset, pos.y + 3);
	return element;
}

void
CBlockFactory::CreateCounter(npsys::fbd_block_n& n) {
	n.create_locally_with_new_id();
	auto block = new CCounter;
	n->e_block.reset(block);
	block->top = n.get();
	block->Expand(2);
	CreateBlock(block);
	block->MountSlot(CreateInputSlot("IN", n, new CBlockInput));
	block->MountSlot(CreateInputSlot("RST", n, new CBlockInput));
	block->MountSlot(CreateOutputSlot("VALUE", n, new CValue(npsys::variable::VT_WORD)));
	block->variable_ = odb::create_node<npsys::variable_n>(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY);
}

// CElement
CElement::_Iterator* CElement::CreateIterator() {
	return new Iter::NullIterator<CElement*>;
}

bool CElement::HitTest(const D2D1_POINT_2F& pt, UINT& flags) {
	auto hit = false;
	
	if (pt.x > rect_.left && pt.x < rect_.right &&
		pt.y > rect_.top && pt.y < rect_.bottom)
	{
		hit = true;
	}

	if (IsHover() != hit) {
		SetHover(hit);
		flags = EL_INVALIDATE;
	}

	return hit;
}

void CElement::Move(float x, float y) { 
	float width = rect_.right - rect_.left;
	float height = rect_.bottom - rect_.top;
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + height;
}

void CElement::MoveDxDy(float dx, float dy) { 
	rect_.left += dx;
	rect_.top += dy;
	rect_.right += dx;
	rect_.bottom += dy;
}

MoveCommand* CElement::CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) { 
	return new MoveCommand(this, delta, qtRoot); 
}

void CElement::FillPropertyGrid(CPropertyGrid* grid) noexcept {
	ATLASSERT(grid);
	grid->AddVariantItem(PRB::ID_SECTION_GENERAL, "Name", GetName(), PRB::ID_PR_NAME);
	grid->AddVariantItem(PRB::ID_SECTION_POSITION, "X", rect_.left, PRB::ID_PR_POSITION_X, false);
	grid->AddVariantItem(PRB::ID_SECTION_POSITION, "Y", rect_.top, PRB::ID_PR_POSITION_Y, false);
}

void CElement::OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept {

}

// CElementManipulator
S_CURSOR CElementManipulator::MouseButtonDown(CPoint point, CAlgorithmView* pWnd) {
	const D2D1::Matrix3x2F& invM = pWnd->GetGraphics()->GetInverseMatrix();
	const D2D1::MyPoint2F& pos = static_cast<D2D1::MyPoint2F&>(
		const_cast<D2D1_POINT_2F&>(element_->GetPosition()));

	_oldpos = pos;
	D2D1::MyPoint2F m_ptCursor = D2D1::Point2F((float)point.x, (float)point.y) * invM;
	_offset = m_ptCursor - _oldpos;
	was_draged_ = false;

	return S_CURSOR::A_ONE_ELEM;
}

void CElementManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	const D2D1::Matrix3x2F& invM = pWnd->GetGraphics()->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pWnd->GetGraphics()->GetMatrix();
	const float& scale_f = matrix._11;

	D2D1::MyPoint2F pt { 
		static_cast<float>(point.x) - _offset.x * scale_f,
		static_cast<float>(point.y) - _offset.y * scale_f };
	
	if (!was_draged_) {
		was_draged_ = true;
		pWnd->SetCursor(AlphaCursor::Drag);
		RemoveFromQuadrant(element_, &pWnd->qtRoot);
		pWnd->GetGraphics()->SetEditingMode(true);
		element_->BeginEditState();
		return;
	}

	pt = pt * invM;
	element_->Move(
		pWnd->CalcNearestPos(pt.x), 
		pWnd->CalcNearestPos(pt.y)
	);
	pWnd->Invalidate();
}

void CElementManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	if (!was_draged_) return;

	element_->EndEditState();

	PasteToQuadrant(element_, &pWnd->qtRoot);

	auto editor = pWnd->alg_->GetBlocks();
	const D2D1::MyPoint2F& pos = static_cast<D2D1::MyPoint2F&>(
		const_cast<D2D1_POINT_2F&>(element_->GetPosition()));
	float dx = pos.x - _oldpos.x;
	float dy = pos.y - _oldpos.y;

	if (fabs(dx) > 0.001f || fabs(dy) > 0.001f) {
		editor->AddCommand(
			element_->CreateMoveCommand(D2D1::Point2F(dx, dy), &pWnd->qtRoot), false);
	}
	pWnd->GetGraphics()->SetEditingMode(false);
	pWnd->Invalidate();
}

void CElementManipulator::Draw(CGraphics* pGraphics) {
	if (was_draged_) element_->Draw(pGraphics);
}


void CSliderManipulator::Drag(CPoint screen_point, CAlgorithmView* pWnd) {
	const D2D1::Matrix3x2F& invM = pWnd->GetGraphics()->GetInverseMatrix();
	const D2D1::Matrix3x2F& matrix = pWnd->GetGraphics()->GetMatrix();
	const float& scale_f = matrix._11;
	const float& x_translate_f = matrix._31;
	
	D2D1::MyPoint2F pt { 
		static_cast<float>(screen_point.x) - _offset.x * scale_f,
		static_cast<float>(screen_point.y) - _offset.y * scale_f };
	
	pt = pt * invM;

	auto rect = slider_->GetBounds();
	auto x_left_lim = rect.left + constants::slider::thing::offset, 
		x_right_lim = rect.right - constants::slider::thing::offset - constants::slider::thing::width;

	if (!was_draged_) {
		was_draged_ = true;
		pWnd->SetCursor(AlphaCursor::Drag);
		RemoveFromQuadrant(element_, &pWnd->qtRoot);
		element_->BeginEditState();
		return;
	}

	if (pt.x < x_left_lim) pt.x = x_left_lim;
	if (pt.x > x_right_lim) pt.x = x_right_lim;

	auto [x, y] = element_->GetPosition();

	auto length = std::abs(pt.x - x);

	bool step = length > constants::slider::SLIDER_STEP / 2.0f;
	
	if (!step) return;
	
	float mul = std::floor(length / constants::slider::SLIDER_STEP);
	if (mul < 1.0f) mul = 1.0f;

	if (pt.x < x) {
		x -= mul * constants::slider::SLIDER_STEP;
	} else {
		x += mul * constants::slider::SLIDER_STEP;
	}

	element_->Move(x, y);
	
	pWnd->Invalidate();
}


// CGeometryElement
CGeometryElement::CGeometryElement() {
	m_matrixTransform = D2D1::Matrix3x2F::Identity();
}
CGeometryElement::CGeometryElement(CGeometryElement& el) : 
	CElement(el) {
	m_matrixTransform = D2D1::Matrix3x2F::Identity();
	m_colorFill = el.m_colorFill;
	m_colorStroke = el.m_colorStroke;
}
void CGeometryElement::Move(float x, float y) {
	float width = rect_.right - rect_.left;
	float height = rect_.bottom - rect_.top;

	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + height;

	m_matrixTransform = D2D1::Matrix3x2F::Translation(x, y);
	UpdateTransformedGeometry();
}
void CGeometryElement::MoveDxDy(float dx, float dy) {
	rect_.left += dx;
	rect_.top += dy;
	rect_.right += dx;
	rect_.bottom += dy;
	m_matrixTransform = m_matrixTransform * D2D1::Matrix3x2F::Translation(dx, dy);
	UpdateTransformedGeometry();
}
void CGeometryElement::UpdateTransformedGeometry() {
	ASSERT(m_geometry);
	dis::Get().d2dFactory->CreateTransformedGeometry(m_geometry.Get(), 
		m_matrixTransform, m_geometryTransformed.ReleaseAndGetAddressOf());
	
	SetGeometryRealizationFlag(true);
}

bool CGeometryElement::HitTest(const D2D1_POINT_2F& pt, UINT& flags) {
	ASSERT(m_geometryTransformed);

	bool hit; 
	{
		BOOL tmp;
		m_geometryTransformed->FillContainsPoint(pt, nullptr, &tmp);
		hit = static_cast<bool>(tmp);
	}

	if (IsHover() != hit) {
		SetHover(hit);
		flags = EL_INVALIDATE;
	}

	return hit;
}
void CGeometryElement::Transform(const D2D1::Matrix3x2F& matrix) {
	m_matrixTransform = m_matrixTransform * matrix;
	UpdateTransformedGeometry();
}

void CGeometryElement::Draw(CGraphics* graphics) {
	auto const normalMode = !graphics->IsEditingMode();
	auto const colorStroke = (IsSelected() ? SolidColor::SelColor : (IsHover() ? SolidColor::SliderThingHovered : m_colorStroke));
	auto const geometry_dirty = IsGeometryRealizationDirty();

	if (normalMode && geometry_dirty) {
		graphics->CreateFilledGeometryRealization(
			m_geometryTransformed.Get(), m_geometryRealizationFilled.ReleaseAndGetAddressOf());
		graphics->CreateStrokedGeometryRealization(
			m_geometryTransformed.Get(), 1.0f, m_geometryRealizationStroked.ReleaseAndGetAddressOf());
		SetGeometryRealizationFlag(false);
	}

	if (!geometry_dirty) [[likely]] {
		if (m_colorFill != SolidColor::NoColor) [[likely]] {
			graphics->DrawGeometryRealization(m_geometryRealizationFilled.Get(), m_colorFill);
		}
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

// CGroup
CGroup::~CGroup() {
	for (auto el : elements_) {
		if (el && !el->IsNode()) delete el;
	}
}
CElement::_Iterator* CGroup::CreateIterator() {
	m_iterator.SetRange(elements_.begin(), elements_.end());
	return static_cast<CElement::_Iterator*>(&m_iterator);
}
void CGroup::Move(float x, float y) {
	float dx = x - rect_.left;
	float dy = y - rect_.top;
	CGeometryElement::Move(x, y);
	for (auto& i : elements_) {
		i->MoveDxDy(dx, dy);
	}
}
void CGroup::MoveDxDy(float dx, float dy) {
	CGeometryElement::MoveDxDy(dx, dy);
	for (auto& i : elements_)
		i->MoveDxDy(dx, dy);
}
void CGroup::CreateRgnShape() noexcept {
	dis::Get().d2dFactory->CreateRectangleGeometry(rect_, 
		(ID2D1RectangleGeometry**)m_geometry.GetAddressOf());
}
// CBlock
CBlock::CBlock()
	: manipulator_(this) {
	SetHitAccess();
	//		st_new_block_ = false;
	//		st_name_changed_ = false;
}
CBlock::~CBlock() {
	for (auto& slot : top->slots) {
		slot->e_slot->SetParent(nullptr);
	}
}
void CBlock::SetName(const std::string& name) noexcept {
	if (GetName() != name) {
		top->set_name(name);
//		st_name_changed_ = true;
		GetTextName()->SetText(name);
		for (auto& slot : top->slots) slot->e_slot->ParentNameChanged();
	}
}

void CBlock::UpdateSize() {
	rect_.bottom = rect_.top + constants::block::block_height(
		static_cast<int>(std::max(top->slots.i_size(), top->slots.o_size()))
	);
	CreateRgnShape();
	UpdateTransformedGeometry();
}

void CBlock::Expand(int nCount) {
	rect_.right = rect_.left + constants::block::BLOCK_WIDTH;
	rect_.bottom = rect_.top + constants::block::block_height(nCount);
}

void CBlock::Draw(CGraphics* graphics) {
	if (!IsEditState()) {
		graphics->FillBlock(D2D1::RoundedRect(rect_, constants::block::RADIUS_X, constants::block::RADIUS_Y), m_colorIndex);
	}

	if (IsSelected()) {
		graphics->DrawRoundRect(D2D1::RoundedRect(rect_, constants::block::RADIUS_X, constants::block::RADIUS_Y), SolidColor::SelColor, 0.8f);
	} else if (IsHover()) {
//		pGraphics->DrawRoundRect(D2D1::RoundedRect(rect_, constants::block::RADIUS_X, constants::block::RADIUS_Y), Color::DarkGray , 1.0f);
	}
//  Order
//	std::wstring order = std::to_wstring(GetExecuteOrder());
//	pGraphics->DrawTextCenter(
//		order.c_str(),
//		order.length(),
//		D2D1::RectF(rect_.left - 15, rect_.top, rect_.left, rect_.top + 16),
//		Color::Black);
}
void CBlock::CreateRgnShape() noexcept {
	dis::Get().d2dFactory->CreateRoundedRectangleGeometry(
		D2D1::RoundedRect(D2D1::RectF(0.0f, 0.0f, rect_.right - rect_.left, rect_.bottom - rect_.top), constants::block::RADIUS_X, constants::block::RADIUS_Y),
		(ID2D1RoundedRectangleGeometry**)m_geometry.ReleaseAndGetAddressOf());
}
Command* CBlock::DeleteElement(CBlockComposition* editor, MyQuadrantNode* qtRoot) {
	auto* group_cmd = new GroupCommand<Command>;
	for (auto& s : top->slots) {
		for (CSlot::line_iterator it = s->e_slot->begin(); it != s->e_slot->end();) {
			group_cmd->AddCommand(new DeleteLineCommand((*it), editor, qtRoot), true);
			it = s->e_slot->begin();
		}
	}
	group_cmd->AddCommand(new DeleteBlockCommand(top->self_node.fetch(), editor, qtRoot), false);
	return group_cmd;
}
std::vector<CLine*> CBlock::GetLines() const {
	std::vector<CLine*> lines;
	for (auto& slot : top->slots) {
		for (auto line : *slot->e_slot) {
			lines.push_back(line);
		}
	}
	return lines;
}
void CBlock::CollectLines(std::vector<CLine*>& lines) const {
	for (auto& slot : top->slots) {
		for (auto line : *slot->e_slot) {
			if (std::find(lines.begin(), lines.end(), line) != lines.end()) continue;
			lines.push_back(line);
		}
	}
}
void CBlock::UpdateBezierLines() {
	for (auto& s : top->slots) {
		for (auto& l : *s->e_slot) {
			l->Create();
		}
	}
}
void CBlock::DeletePermanent() {
	for (auto& s : top->slots) {
		s->e_slot->DeletePermanent();
		
		auto it = std::find(elements_.begin(), elements_.end(), s->e_slot.get());
		ASSERT(it != elements_.end());
		(*it) = nullptr;
	}
	top->slots.clear_with_nodes();
}

size_t CBlock::InputHasValidVariable() noexcept {
	size_t count = 0;
	for (auto it = top->slots.begin_in(); it != top->slots.end_in(); it++)
		count += (static_cast<CInputSlot*>((*it)->e_slot.get())->GetInputVariable() != nullptr);
	return count;
}

MoveCommand* 
CBlock::CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) {
	return new BlockMoveCommand(this, delta, qtRoot);
}

void CBlock::FillPropertyGrid(CPropertyGrid* grid) noexcept {
	ATLASSERT(grid);
	grid->AddVariantItem(PRB::Section::ID_SECTION_GENERAL,
		"Id", top->self_node.id(), 0, false);
	CElement::FillPropertyGrid(grid);
	if (prop_list_.size()) {
		g_pPropertyGrid->AddSection(PRB::ID_SECTION_BLOCK_SPECIAL, "Special");
		for (auto& i : prop_list_) i->FillPropertyGrid(grid);
	}
}

void CBlock::OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept {	
	auto bvar = (CPropertyGrid::Variant*)(wParam);
	auto var = boost::get<npsys::CMyVariant>(*bvar);
	reinterpret_cast<BlockProperty*>(lParam)->OnPropertyChanged(var);
	top->set_modified();
}

// CBlockManipulator
void CBlock::CBlockManipulator::Drag(CPoint point, CAlgorithmView* pWnd) {
	if (!was_draged_) {
		m_lines = std::move(static_cast<CBlock*>(element_)->GetLines());
		for (auto& l : m_lines) pWnd->qtRoot.DeleteElement(l);
	}
	CElementManipulator::Drag(point, pWnd);
	for (auto& l : m_lines) l->Create();
}

void CBlock::CBlockManipulator::MouseButtonUP(CPoint point, CAlgorithmView* pWnd) {
	if (was_draged_) {
		for (auto& l : m_lines) pWnd->qtRoot.AddElement(l);
		CElementManipulator::MouseButtonUP(point, pWnd);
	}
}

void CBlock::CBlockManipulator::Draw(CGraphics* pGraphics) {
	if (!was_draged_) return;
	element_->Draw(pGraphics);
	for (auto& l : m_lines) l->Draw(pGraphics);
}

// CInput
void CInput::SetName(const std::string& name) noexcept {
	CBlock::SetName(name);
	AdjustBlock();
}

void CInput::MountSlot(npsys::fbd_slot_n&& slot) {
	top->set_modified();
	slot->set_modified();
	auto e_slot = slot->e_slot.get();
	e_slot->SetParent(this);
	elements_.push_back(e_slot);
	top->slots.add_output_slot(slot);
}

void CInput::AdjustBlock() noexcept {
	D2D1_RECT_F text_rect;
	GetTextName()->GetRect(text_rect);
	
	auto old_right = rect_.right;
	rect_.right = text_rect.right + constants::block::SLOT_TEXT + constants::block::SLOT_SPACE * 2.0f + constants::block::SLOT_SIZE;
	
	CreateRgnShape();
	UpdateTransformedGeometry();
	
	if (std::abs(old_right - rect_.right) > 0.001f) {
		GetSlot()->SetOutputSlotPosition(rect_);
		GetSlot()->top->set_modified(true);
	}
}

// COutput
void COutput::SetName(const std::string& name) noexcept {
	CBlock::SetName(name);
	AdjustBlock();
}

void COutput::MountSlot(npsys::fbd_slot_n&& slot) {
	top->set_modified();
	slot->set_modified();
	auto e_slot = slot->e_slot.get();
	e_slot->SetParent(this);
	elements_.push_back(e_slot);
	top->slots.add_input_slot(slot);
}

void COutput::AdjustBlock() noexcept {
	D2D1_RECT_F text_rect;
	GetTextName()->GetRect(text_rect);

	auto old_left = rect_.left;
	rect_.left = text_rect.left - (constants::block::SLOT_TEXT - 12.0f) - constants::block::SLOT_SPACE * 2.0f - constants::block::SLOT_SIZE;
	
	CreateRgnShape();
	UpdateTransformedGeometry();

	if (std::abs(old_left - rect_.left) > 0.001f) {
		GetSlot()->SetInputSlotPosition(rect_);
		GetSlot()->top->set_modified(true);
	}
}

void CDelay::CreateInplaceProperties() noexcept {
	D2D1_RECT_F rc;
	rc.left = rect_.left + 10.0f;
	rc.top = rect_.top + constants::block::y_pos(1);
	rc.right = rect_.right - 10.0f;
	rc.bottom = rect_.top + constants::block::y_pos(2);
	AddGraphicElement(new CInplaceBlockProperty(*prop_list_[0].get(), rc));
}

// CPID


// CTime
void CTime::HardwareSpecific_Init(npsys::CNetworkDevice* device) {
	auto const& map = device->InitInternalBlock(this);
	
	for (size_t slot_index = 0, ix = 0; slot_index <= 6; ++slot_index, ++ix) {
		Hardware_InitInternal(CSlot::OUTPUT_SLOT, slot_index, std::get<0>(map[ix]), std::get<1>(map[ix]), device->self_node);
	}
}

void CTime::HardwareSpecific_Clear() {
	auto& slots = top->slots;
	for (size_t i = 0; i <= 6; ++i) {
		o_at(i).GetVariableAsNode()->remove();
	}
	top->self_node.fetch().store();
}

// CSliderThing
CSliderThing::CSliderThing() 
	: manipulator_(this, nullptr) { 

	SetHitAccess(); 
	m_colorFill = SolidColor::SliderThing;
}

CSliderThing::CSliderThing(CScheduleSlider* parent) 
	: manipulator_(this, parent)
	, parent_(parent)
{
	rect_.right = constants::slider::thing::width;
	rect_.bottom = constants::slider::thing::height;
	
	SetZOrder(4);
	SetHitAccess();

	m_colorFill = SolidColor::SliderThing;
}

void CSliderThing::CreateRgnShape() noexcept {
	m_geometry = dis::Get().slider_thing;
}

CMenuHandle CSliderThing::GetContextMenu() {
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_REMOVE_SLIDER_THING, L"Remove");
	return menu.Detach();
}

// CSliderTimeChart
CSliderTimeChart::CSliderTimeChart() {
	rect_.right = constants::slider::SCHEDULE_BLOCK_TIME_CHART_WIDTH;
	rect_.bottom = 20;
	SetZOrder(3);

	m_colorStroke = SolidColor::Black;
}

void CSliderTimeChart::CreateRgnShape() noexcept {
	m_geometry = dis::Get().slider_time_chart;
}

// CScheduleSlider
CScheduleSlider::CScheduleSlider() {
	rect_.right = constants::slider::SLIDER_WIDTH;
	rect_.bottom = constants::slider::SLIDER_HEIGHT;
	
	SetZOrder(2);
	SetHitAccess();

	m_colorFill = SolidColor::SliderBackground;
	m_colorStroke = SolidColor::DarkGray;
}

void CScheduleSlider::CreateRgnShape() noexcept {
	dis::Get().d2dFactory->CreateRoundedRectangleGeometry(
		D2D1::RoundedRect(D2D1::RectF(0.0f, 0.0f, rect_.right - rect_.left, rect_.bottom - rect_.top), 3, 3),
		(ID2D1RoundedRectangleGeometry**)m_geometry.ReleaseAndGetAddressOf()
	);
}

CMenuHandle CScheduleSlider::GetContextMenu() {
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SLIDER_ADD_TIME_POINT, L"Add Time Point");
	return menu.Detach();
}
CSliderTimeChart* CScheduleSlider::GetTimeChart() { 
	return static_cast<CSliderTimeChart*>(elements_[0]); 
}
CScheduleSlider::SliderInfo CScheduleSlider::GetSliderInfo() {
	SliderInfo info;

	info.length = GetTimeChart()->Width();
	auto left = GetTimeChart()->GetBounds().left;

	for (size_t i = 1; i < elements_.size(); ++i) {
		auto const rect = elements_[i]->GetBounds();
		info.positions.emplace_back((rect.left + (rect.right - rect.left) / 2.0f) - left);
	}

	return info;
}

// CBlockSchedule
void CBlockSchedule::HardwareSpecific_Init(npsys::CNetworkDevice* device) {
	auto const& map = device->InitInternalBlock(this);
	
	for (size_t slot_index = 1, ix = 0; slot_index <= 3; ++slot_index, ++ix) {
		Hardware_InitInternal(CSlot::OUTPUT_SLOT, slot_index, std::get<0>(map[ix]), std::get<1>(map[ix]), device->self_node);
	}
}

void CBlockSchedule::HardwareSpecific_Clear() {
	auto& slots = top->slots;
	for (size_t i = 1; i <= 3; ++i) {
		o_at(i).GetVariableAsNode()->remove();
	}
	top->self_node.fetch().store();
}

CScheduleSlider* CBlockSchedule::GetSlider() { 
	return static_cast<CScheduleSlider*>(FirstCustomElement()); 
}

// CParameter
CParameter::CParameter() {
	rect_.left = 0;
	rect_.top = 0;
	rect_.right = constants::block::PARAM_WIDTH;
	rect_.bottom = constants::block::PARAM_HEIGHT;
}

void CParameter::Draw(CGraphics* pGraphics) {
	if (!IsEditState()) {
		if (quality_) {
			pGraphics->FillBlock(D2D1::RoundedRect(rect_, 
				constants::block::RADIUS_X, constants::block::RADIUS_Y), 
				BlockColor::ParameterQuality, false);
		} else {
			pGraphics->FillBlock(D2D1::RoundedRect(rect_, 
				constants::block::RADIUS_X, constants::block::RADIUS_Y), 
				BlockColor::Parameter, false);
		}
	}

	if (IsSelected()) 
		pGraphics->DrawRoundRect(D2D1::RoundedRect(rect_, constants::block::RADIUS_X, constants::block::RADIUS_Y), SolidColor::SelColor, 1.0f);
//	else pGraphics->DrawRoundRect(D2D1::RoundedRect(rect_, RADIUS_X, RADIUS_Y), Color::Black);
	//Order

//	std::wstring order = std::to_wstring(GetExecuteOrder());
//	pGraphics->DrawTextCenter(
//		order.c_str(),
//		order.length(),
//		D2D1::Rect(rect_.left - 15, rect_.top, rect_.left, rect_.top + 10),
//		Color::Black);

	if (m_paramType == PARAMETER_TYPE::P_INTERNAL_REF || m_paramType == PARAMETER_TYPE::P_EXTERNAL_REF) {
		if (link_name_dirty_ == true) {
			auto slot_type = static_cast<CReference*>(GetSlot()->GetSlotType());
			auto str = wide(slot_type->GetLinkName(&link_name_dirty_));

			HR(dis::Get().pDWriteFactory->CreateTextLayout(str.c_str(), static_cast<int>(str.length()),
				dis::Get().pTextFormatLeft,
				500, 10,
				link_text_layout_.ReleaseAndGetAddressOf()));

			DWRITE_TEXT_METRICS metrics;
			link_text_layout_->GetMetrics(&metrics);
			link_text_width_ = metrics.width / 2.0f;
			link_text_layout_->SetMaxWidth(metrics.width);
			link_text_layout_->SetMaxHeight(metrics.height);
			
			link_name_dirty_ = false;
		}

		pGraphics->DrawTextLayout({ rect_.left + (rect_.right - rect_.left) / 2.0f - link_text_width_, rect_.bottom + 10.0f },
			link_text_layout_.Get(),
			SolidColor::Black);
	}
}

void CBlock::PrintDebugTooltip(std::stringstream& ss) {
	ss << "block_id - " << top->self_node.id() << "\n";
}

void CBlockSchedule::PrintDebugTooltip(std::stringstream& ss) {
	CBlock::PrintDebugTooltip(ss);
	for (auto& v : variables_) {
		ss << *v.get() << "\n";
	}
}

void CParameter::PrintDebugTooltip(std::stringstream& ss) {
	CBlock::PrintDebugTooltip(ss);
	GetSlot()->GetSlotType()->PrintDebugTooltip(ss);
}

bool CParameter::ShowPropertiesDialog(CBlockComposition* blocks, npsys::algorithm_n& alg) {
	CDlg_BlockParam dlg(this, alg);
	dlg.DoModal();
	return true;
}
bool CParameter::ShowOnlineDialog(npsys::CAlgorithmExt* alg) {
//	if (GetSlot()->OnlineGetValue().is_discrete()) {
//		CDlg_OnlineDiscrete dlg(this, alg);
//		dlg.DoModal(g_hMainWnd);
//	} else {
		CDlg_OnlineValue dlg(this, alg);
		dlg.DoModal(g_hMainWnd);
//	}
	return true;
}

PARAMETER_TYPE CParameter::GetParamType() {
	return m_paramType;
}

PARAMETER_TYPE CParameter::SetParamType(PARAMETER_TYPE paramType) {
	PARAMETER_TYPE old = m_paramType;
	m_paramType = paramType;
	link_name_dirty_ = true;
	top->set_modified();
	return old;
}

void CParameter::FillPropertyGrid(CPropertyGrid* grid) noexcept {
	grid->AddVariantItem(PRB::Section::ID_SECTION_GENERAL,
		"Id", top->self_node.id(), 0, false);
	CElement::FillPropertyGrid(grid);
	GetSlot()->GetSlotType()->FillPropertyGrid(grid);
}

void CParameter::OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept {
	switch (lParam)
	{
	case PRB::ID_PR_DEFAULT_VALUE:
		GetSlot()->GetSlotType()->OnPropertyChanged(wParam, lParam);
		break;
	default:
		break;
	}
}

void CBlock::Hardware_InitInternal(CSlot::SLOT_DIRECTION slot_dir, 
	size_t slot_index, int var_type, uint16_t addr, odb::weak_node<npsys::device_n> dev) 
{
	auto& slots = top->slots;
	npsys::fbd_slot_n slot;

	if (slot_dir == CSlot::SLOT_DIRECTION::OUTPUT_SLOT) {
		auto it = slots.begin_out();
		std::advance(it, slot_index);
		slot = (*it);
	} else {
		auto it = slots.begin_in();
		std::advance(it, slot_index);
		slot = (*it);
	}
	
	auto slot_type = 
		dynamic_cast<CFixedValue*>(slot->e_slot->GetSlotType());

	ASSERT(slot_type);

	if (slot_type->GetVariable() != nullptr) return;

	slot_type->Hardware_InitVariable(dev, var_type, addr);
	
	slot.store();
}

PARAM_SCOPE CParameter::GetScope() {
	return GLOBAL;
}
CMenuHandle CParameter::GetContextMenu() {
	auto menu = CBlock::GetContextMenu();
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_AE_COPY_AS_LINK, L"Copy As Link");
	menu.AppendMenu(MF_STRING, ID_AE_COPY_SLOT_PATH_TO_CB, L"Copy Full Path to Clipboard");
	return menu.Detach();
}
// CInaccessibleParameter
PARAM_SCOPE CInaccessibleParameter::GetScope() {
	return INTERNAL;
}

// CDynamicBlock
void CDynamicBlock::ExpandReduceInputs(size_t new_count, CBlockFactory& factory) {
	auto inputs_size = this->top->slots.i_size();
	auto block_node = this->top->self_node.fetch();

	if (inputs_size == new_count) {
		return;
	} else if (inputs_size > new_count) {
		// can't reduce slots for now
	} else {
		for (size_t ix = inputs_size; ix < new_count; ++ix) {
			this->MountSlot(factory.CreateInputSlot("IN_" + std::to_string(ix + 1), block_node, new CBlockInput));
		}
		UpdateSize();
	}
}
CMenuHandle CDynamicBlock::GetContextMenu() {
	CMenuHandle menu = CElement::GetContextMenu();
	menu.InsertMenu(ID_EDIT_CUT, MF_SEPARATOR);
	menu.InsertMenu(0, MF_STRING, ID_EXPAND_INPUTS, L"Expand Inputs");
	return menu.Detach();
}
// CTextElement
CTextElement::CTextElement() 
	: _manipulator(this) {
	SetZOrder(4);
	m_align = 0;
}

CTextElement::CTextElement(std::string&& text, float x, float y, int align, FONT_SIZE font_size)
	: text_(std::move(text))
	, m_align(align)
	, font_size_(font_size)
	, _manipulator(this) {
	SetZOrder(4);
	OnTextChanged();
	SetPosition(x, y, align);
}

void CTextElement::UpdatePosition() {
	DWRITE_TEXT_METRICS metrics;
	textLayout_->GetMetrics(&metrics);
	
	textLayout_->SetMaxWidth(metrics.width);
	textLayout_->SetMaxHeight(metrics.height);

	if (m_align & T_LEFT) {
		rect_.right = rect_.left + metrics.width;
		rect_.bottom = rect_.top + metrics.height;
	} else if (m_align & T_RIGHT) {
		rect_.left = rect_.right - metrics.width;
		rect_.top = rect_.bottom - metrics.height;
	} else if (m_align & T_CENTER) {
		D2D1_POINT_2F ptCenter;
		ptCenter.x = rect_.left + (rect_.right - rect_.left) / 2;
		ptCenter.y = rect_.top + (rect_.bottom - rect_.top) / 2;
		rect_.left = ptCenter.x - metrics.width / 2;
		rect_.top = ptCenter.y - metrics.height / 2;
		rect_.right = ptCenter.x + metrics.width / 2;
		rect_.bottom = ptCenter.y + metrics.height / 2;
	} else {
		rect_.right = metrics.width;
		rect_.bottom = metrics.height;
	}
}

void CTextElement::OnTextChanged() {
	auto str = wide(text_);

	HR(dis::Get().pDWriteFactory->CreateTextLayout(
		str.c_str(),
		static_cast<int>(str.length()),
		dis::Get().text_centered[font_size_],
		500, 10,
		textLayout_.ReleaseAndGetAddressOf()));

	UpdatePosition();
}

void CTextElement::Draw(CGraphics* pGraphics) {
	pGraphics->DrawTextLayout((D2D1::MyPoint2F&)rect_, textLayout_.Get(), SolidColor::Black);
}

void CTextElement::SetPosition(float x, float y, int align) {
	ASSERT(textLayout_);
	
	m_align = align;

	float width = rect_.right - rect_.left;
	float height = rect_.bottom - rect_.top;

	if (align & T_LEFT) {
		rect_.left = x;
		rect_.right = x + width;
		if (align & T_VCENTER) {
			float h_height = height / 2.0f;
			rect_.top = y - h_height;
			rect_.bottom = y + h_height;
		} else {
			rect_.top = y;
			rect_.bottom = y + height;
		}
	} else if (align & T_RIGHT) {
		rect_.left = x - width;
		rect_.right = x;
		if (align & T_VCENTER) {
			float h_height = height / 2.0f;
			rect_.top = y - h_height;
			rect_.bottom = y + h_height;
		} else {
			rect_.top = y;
			rect_.bottom = y + height;
		}
	} else if (align & T_CENTER) {
		float h_width = width / 2.0f;
		rect_.left = x - h_width;
		rect_.right = x + h_width;
		if (align & T_VCENTER) {
			float h_height = height / 2.0f;
			rect_.top = y - h_height;
			rect_.bottom = y + h_height;
		} else {
			rect_.top = y;
			rect_.bottom = y + height;
		}
	}
}

// CStaticTextElement
std::map<std::string, wrl::ComPtr<IDWriteTextLayout>> CStaticTextElement::m_layouts;

CStaticTextElement::~CStaticTextElement() {
	if (textLayout_.Reset() == 0)
		m_layouts.erase(text_);
}

void CStaticTextElement::OnTextChanged() {
	auto str = wide(text_);
	if (m_layouts.find(text_) == m_layouts.end()) {
		HR(dis::Get().pDWriteFactory->CreateTextLayout(
			str.c_str(),
			static_cast<int>(str.length()),
			dis::Get().text_centered[font_size_], 
			500, 10,
			textLayout_.ReleaseAndGetAddressOf()));
		m_layouts[text_] = textLayout_;
	} else {
		textLayout_ = m_layouts[text_];
	}
	UpdatePosition();
}

// CDxEdit
void CDxEdit::SetText(const std::string& text) {
	text_ = text;
	auto str = wide(text);
	HR(dis::Get().pDWriteFactory->CreateTextLayout(
		str.c_str(),
		static_cast<int>(str.length()),
		dis::Get().pTextFormatLeft,
		500, 10,
		text_layout_.ReleaseAndGetAddressOf()));

	DWRITE_TEXT_METRICS metrics;
	text_layout_->GetMetrics(&metrics);

	text_layout_->SetMaxWidth(metrics.width);
	text_layout_->SetMaxHeight(metrics.height);
}

void CDxEdit::Draw(CGraphics* pGraphics) {
	D2D1_ROUNDED_RECT rc;
	rc.rect = rect_;
	rc.radiusX = rc.radiusY = 3.0f;
	
	pGraphics->FillRoundRect(rc, SolidColor::EditColor);

	if (IsHover() == TRUE) {
		pGraphics->DrawRoundRect(rc, SolidColor::Blue, 0.7f);
	} else {
		pGraphics->DrawRoundRect(rc, SolidColor::Gray, 0.5f);
	}

	pGraphics->PushClip(rect_);

	auto pt = (D2D1::MyPoint2F&)rect_;
	pt.x += 3.0f;

	pGraphics->DrawTextLayout(pt, text_layout_.Get(), SolidColor::Black);
	pGraphics->PopClip();
}

CInputSlot* CConfigurableBlock::GetInput(const std::string_view slot_name) {
	for (auto it = top->slots.begin_in(); it != top->slots.end_in(); it++) {
		if ((*it)->get_name() == slot_name) return static_cast<CInputSlot*>((*it)->e_slot.get());
	}
	return nullptr;
}

COutputSlot* CConfigurableBlock::GetOutput(const std::string_view slot_name) {
	for (auto it = top->slots.begin_out(); it != top->slots.end_out(); it++) {
		if ((*it)->get_name() == slot_name) return static_cast<COutputSlot*>((*it)->e_slot.get());
	}
	return nullptr;
}

void CConfigurableBlock::AddDisabledSlot(npsys::fbd_slot_n&& slot) {
	disabled_slots_.push_back(std::move(slot));
}

CMenuHandle CConfigurableBlock::GetContextMenu() {
	auto menu = CBlock::GetContextMenu();
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CONFIGURABLE_BLOCK_EDIT, L"Edit block");
	return menu.Detach();
}

void CConfigurableBlock::EnableSlot(npsys::fbd_slot_n& slot_node) {
	auto founded = std::find(disabled_slots_.begin(), disabled_slots_.end(), slot_node);
	ASSERT(founded != disabled_slots_.end());
	MountSlot(std::move(*founded));
	disabled_slots_.erase(founded);
	UpdateSize();
	top->set_modified();
}

void CConfigurableBlock::DisableSlot(npsys::fbd_slot_n& slot_node) {
	auto e_slot = slot_node->e_slot.get();
	auto founded = std::find(elements_.begin(), elements_.end(), e_slot);
	ASSERT(founded != elements_.end());
	elements_.erase(founded);
	
	disabled_slots_.push_back(slot_node);
	auto it = top->slots.erase(slot_node);

	npsys::SlotList::iterator end;
	if (e_slot->GetDirection() == CSlot::INPUT_SLOT) {
		end = top->slots.end_in();
	} else {
		end = top->slots.end_out();
	}

	for (auto index = e_slot->GetSlotIndex(); it != end; it++, index++) {
		(*it)->e_slot->SetSlotPosition(index, rect_.left, rect_.top, rect_.right - rect_.left);
		(*it)->set_modified();
	}

	UpdateSize();
	top->set_modified();
}

// DatabaseAction_ConfigurableSlotRemoved
void DatabaseAction_ConfigurableSlotRemoved::Execute() noexcept {
	auto var_ptr = slot_->e_slot->GetVariableAsNode();
	(*var_ptr)->OwnerRelease(nullptr);
	(*var_ptr) = odb::create_node<npsys::variable_n>((*var_ptr)->GetType());
	(*var_ptr).store();
	slot_.store();
}

CMenuHandle CBlockFunction::GetContextMenu() {
	auto menu = CBlock::GetContextMenu();
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CONFIGURABLE_BLOCK_EQUATION, L"Equation");
	return menu.Detach();
}