// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "module.h"
#include <npsys/variable.h>

namespace npsys {

int CI2CModuleSegment::GetLenght() const {
	using Variable = npsys::variable;
	int len = 0;
	for (auto& i : values) {
		len += (variable::GetSize(i->GetType())) + ((i->GetType() & Variable::VQUALITY) ? 1 : 0);
	}
	return len;
}

void CI2CModuleSegment::SetOffset(uint16_t offset) {
	offset_ = offset;
	Recalc(values.begin(), values.end());
}

void CI2CModuleSegment::Recalc(i2c_seg_val_l::iterator first, i2c_seg_val_l::iterator last) {
	if (first == last) return;
	if (first == values.begin()) {
		(*first)->SetAddress(offset_);
	}
	auto prev = first++;
	for (; first != last; first++) {
		(*first)->SetAddress((*prev)->GetAddress() + variable::GetSize((*prev)->GetType())
			+ variable::IsQuality((*prev)->GetType()));
		prev = first;
	}
}

void CI2CModuleSegment::RecalcFrom(CI2CModuleSegmentValue* from) noexcept {
	auto founded = std::find_if(values.begin(), values.end(), [from](const auto& ptr) {
		return from == ptr.get();
	});
	Recalc(founded, values.end());
}

void CI2CModuleSegment::RecalcAll() noexcept {
	Recalc(values.begin(), values.end());
}

void CI2CModuleSegmentValue::AssignedData::Release() noexcept {
	if (status_ != io::Status::relevant) return;

	status_ = io::Status::not_relevant;
	ASSERT_FETCH(var);
	var->OwnerRelease(nullptr);
	var.store();
	var = {};
}

void CI2CModuleSegmentValue::SetAddress(int addr) noexcept {
	if (addr_ == addr) return;
	addr_ = addr;
	if (ass) ass->Release();
}

void CI2CModuleSegmentValue::SetType(int type) noexcept {
	if (type_ == type) return;
	type_ = type;
	if (ass) ass->Release();
}

CI2CModuleSegmentValue* CI2CModuleSegment::CreateNewSegmentValue(const std::string& name, bool assigned) noexcept {
	int offset;
	if (values.empty()) {
		offset = GetOffset();
	} else {
		const auto& back = values.back();
		offset = back->GetAddress() + npsys::variable::GetSize(back->GetType()) 
			+ variable::IsQuality(back->GetType());
	}
	auto ptr = std::make_unique<CI2CModuleSegmentValue>(name, offset, this, assigned);
	auto raw = ptr.get();
	values.push_back(std::move(ptr));
	return raw;
}

} // namespace npsys