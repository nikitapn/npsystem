// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "block.h"
#include "codegenerator.h"
#include "genhelper.h"

void CTypeDeterminant::Reset() {}
void CTypeDeterminant::Generate(COr*) {}
void CTypeDeterminant::Generate(CAnd*) {}
void CTypeDeterminant::Generate(CRsTrigger*) {}
void CTypeDeterminant::Generate(CNot*) {}
void CTypeDeterminant::Generate(CPositiveEdge*) {}
void CTypeDeterminant::Generate(CCounter*) {}
void CTypeDeterminant::Generate(CNegativeEdge*) {}
void CTypeDeterminant::Generate(CAnyEdge*) {}
void CTypeDeterminant::Generate(CDelay*) {}

void CTypeDeterminant::Generate(CBinaryEncoder* block) {
	auto in_size = block->top->slots.i_size();
	auto& out = block->top->slots.at_o(0);
	if (in_size <= 8) {
		out.SetSlotType(new CValue(variable::VT_BYTE | variable::VQUALITY));
	} else if (in_size > 8 && in_size <= 16) {
		out.SetSlotType(new CValue(variable::VT_WORD | variable::VQUALITY));
	} else {
		out.SetSlotType(new CValue(variable::VT_DWORD | variable::VQUALITY));
	}
}
void CTypeDeterminant::Generate(CBinaryDecoder* decoder) {
	/*
	auto begin = decoder->BeginOutput(), end = decoder->EndOutput();
	for (auto i = begin; i != end; ++i)
		(*i)->SetSlotType(new CValue(Variable::VT_DISCRETE | Variable::VQUALITY));
		*/
}

void CTypeDeterminant::Generate(CPID*) {}

void CTypeDeterminant::Generate(CAlarmHigh*) {}

void CTypeDeterminant::Generate(CAlarmLow*) {}

void CTypeDeterminant::Generate(CAdd* block) {
	auto& slots = block->top->slots;
	const auto in1 = i_at(0).GetInputVariable();
	const auto in2 = i_at(1).GetInputVariable();
	auto& out_slot = o_at(0);
	if (!in1 && !in2) {
		out_slot.SetSlotType(new CValue(variable::VT_UNDEFINE | variable::MUTABLE));
	} else if (!in1 && in2) {
		out_slot.SetSlotType(new CValue(in2->GetType() | variable::MUTABLE));
	} else if (in1 && !in2) {
		out_slot.SetSlotType(new CValue(in1->GetType() | variable::MUTABLE));
	} else {
		auto type = get_output_1(in1->GetType(), in2->GetType());
		out_slot.SetSlotType(new CValue(type | variable::MUTABLE));
	}
}
void CTypeDeterminant::Generate(CSub* block) {
	Generate((CAdd*)block);
}

void CTypeDeterminant::Generate(CMul*) {
	/*
	int in_1, in_2, out;

	auto it = pMul->begin();
	CInputSlot* inS_1 = static_cast<CInputSlot*>((*it++));
	CInputSlot* inS_2 = static_cast<CInputSlot*>((*it++));
	COutputSlot* outS = static_cast<COutputSlot*>(*it);

	in_1 = inS_1->DefineTypeR();
	in_2 = inS_2->DefineTypeR();
	out = outS->DefineTypeR();

	int sign = (Variable::IsSigned(Variable::) | (in_2 & Variable::SIGNED);
	int quality = (in_1 & Variable::VQUALITY) | (in_2 & Variable::VQUALITY);

	int t1 = in_1 & Variable::SIZE_MASK;
	int t2 = in_2 & Variable::SIZE_MASK;

	int t3 = std::max(t1, t2);

	if (t3 == Variable::BIT_VALUE) {
		ASSERT(false);
		out = (t1 & Variable::BIT_VALUE) ? quality | sign | t2 : quality | sign | t1;
	} else if (t3 == Variable::SIZE_8) {
		t3 = Variable::SIZE_16;
	} else if (t3 == Variable::SIZE_16) {
		t3 = Variable::SIZE_32;
	}
	out =  t3 | sign | quality;

	outS->SetSlotType(new CValue(out));
	*/
}

void CTypeDeterminant::Generate(CDiv*) {
	/*
	int in_1, in_2, out;

	auto it = pDiv->begin();
	CInputSlot* inS_1 = static_cast<CInputSlot*>((*it++));
	CInputSlot* inS_2 = static_cast<CInputSlot*>((*it++));
	COutputSlot* outS = static_cast<COutputSlot*>(*it);

	in_1 = inS_1->DefineTypeR();
	in_2 = inS_2->DefineTypeR();
	out = outS->DefineTypeR();

	out = get_output_1(in_1, in_2);
	outS->SetSlotType(new CValue(out));
	*/
}

void CTypeDeterminant::Generate(CTime*) {}

void CTypeDeterminant::Generate(CBlockSchedule* block) {
	using byte = npsys::variable::byte;
	
	auto info = block->GetSlider()->GetSliderInfo();
	auto length = info.length;

	std::vector<std::tuple<byte, byte>> time_points; // hour, minutes
	std::transform(info.positions.begin(), info.positions.end(),
		std::back_inserter(time_points), [length](float flt) {
			auto tm = static_cast<int>(
				std::floor((flt / length) * (constants::slider::SCHEDULE_BLOCK_TIME_MAX_TIME_POINTS + 1.0f))
				);
			constexpr auto intervals = constants::slider::SCHEDULE_BLOCK_TIME_HOUR_INTERVALS;
			return std::make_tuple(tm / intervals, (tm % intervals) * 15);
		});

	if (time_points.size() == block->VarCount() / 2) {
		// ok
	} else if (time_points.size() < block->VarCount() / 2) {
		block->ShrinkMemory(time_points.size() * 2);
		block->top->set_modified();
	} else {
		block->ReleaseMemory();
		for (size_t i = 0; i < time_points.size(); ++i) {
			block->AddVariable(odb::create_node<npsys::variable_n>(npsys::variable::VT_BYTE)); // hour
			block->AddVariable(odb::create_node<npsys::variable_n>(npsys::variable::VT_BYTE)); // minute
		}
		block->top->set_modified();
	}

	std::sort(time_points.begin(), time_points.end());
	
	ASSERT(block->VarCount() / 2 == time_points.size());
	
	for (size_t i = 0; i < time_points.size(); ++i) {
		auto& tm = time_points[i];
		auto& var_hour = block->variables_[2 * i];
		auto& var_minute = block->variables_[2 * i + 1];

		if (std::get<0>(tm) != var_hour->DefaultValue_GetValue().u8) {
			var_hour->DefaultValue_SetValue(std::get<0>(tm));
		}
		
		if (std::get<1>(tm) != var_minute->DefaultValue_GetValue().u8) {
			var_minute->DefaultValue_SetValue(std::get<1>(tm));
		}
	}
}

void CTypeDeterminant::Generate(CComparator* block) {}
void CTypeDeterminant::Generate(CBlockFunction*) {}
void CTypeDeterminant::Generate(CInput* pInput) {}
void CTypeDeterminant::Generate(COutput* pOutput) {}

int CTypeDeterminant::get_output_1(int _t1, int _t2) {
	int quality = (_t1 & variable::VQUALITY) | (_t2 & variable::VQUALITY);
	if (_t1 & variable::FLOAT_VALUE ||
		_t2 & variable::FLOAT_VALUE) {
		return variable::VT_FLOAT | quality;
	}
	int t1 = _t1 & variable::SIZE_MASK;
	int t2 = _t2 & variable::SIZE_MASK;
	int t3 = std::max(t1, t2);
	int sign = (_t1 & variable::SIGNED) | (_t2 & variable::SIGNED);
	if (t3 & variable::BIT_VALUE) {
		return (t1 & variable::BIT_VALUE) 
			? quality | sign | t2 
			: quality | sign | t1;
	}
	return t3 | sign | quality | variable::INT_VALUE;
}