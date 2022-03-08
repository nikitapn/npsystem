// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "error.h"
#include "io_avr.h"
#include "network_ext.h"
#include <npsys/variable.h>
#include <avr_firmware/core.h>


namespace npsys {
using namespace io;
//Port
io_reg_addr CAVRPort::GetControlRegisters(const avrinfo::PeripheralInfo& info, bool ioStart) {
	io_reg_addr res;

	// PORT
	auto letter = name_[4];

	std::string port = "PORT", ddr = "DDR", pin = "PIN";
	port += letter;
	ddr += letter;
	pin += letter;

	res.port = info.GetIoAddr(port.c_str()) - (ioStart ? 0x20 : 0);
	res.port_n = port;

	res.ddr = info.GetIoAddr(ddr.c_str()) - (ioStart ? 0x20 : 0);
	res.ddr_n = ddr;

	res.pin = info.GetIoAddr(pin.c_str()) - (ioStart ? 0x20 : 0);
	res.pin_n = pin;

	ASSERT(res.port != 0xff && res.ddr != 0xff && res.pin != 0xff);

	return res;
}

// CAVRPin

NPSystemIcon CAVRPin::GetIcon() const noexcept {
	switch (use_) {
	case avrinfo::PinPurpose::INPUTPU_PIN:
		return NPSystemIcon::Di_Pin;
	case avrinfo::PinPurpose::INPUT_PIN:
		return NPSystemIcon::Di_Pin;
	case avrinfo::PinPurpose::OUTPUT_PIN:
		return NPSystemIcon::Do_Pin;
	case avrinfo::PinPurpose::ANALOG_PIN:
		return NPSystemIcon::Ai_Pin;
	default:
		return NPSystemIcon::Object;
		break;
	}
}

void CAVRPin::SetPinPurpose(avrinfo::PinPurpose use) noexcept {
	use_ = use;

	switch (use_) {
	case avrinfo::PinPurpose::INPUTPU_PIN:
	case avrinfo::PinPurpose::INPUT_PIN:
	case avrinfo::PinPurpose::ANALOG_PIN:
		type_ = PINTYPE::INPUT;
		break;
	case avrinfo::PinPurpose::OUTPUT_PIN:
		type_ = PINTYPE::OUTPUT;
		break;
	default:
		type_ = PINTYPE::BLOCKED;
		break;
	}
	
	if (status_ == io::Status::assigned) return;
	
	if (last_loaded_use_ == use) status_ = io::Status::relevant;
	else status_ = io::not_relevant;
}

oriandival CAVRPin::GetDDRValImpl(avrinfo::PinPurpose use, int bit) noexcept {
	oriandival val { 0 };
	switch (use) {
	case avrinfo::PinPurpose::INPUTPU_PIN:
	case avrinfo::PinPurpose::INPUT_PIN:
	case avrinfo::PinPurpose::ANALOG_PIN:
		val.andi |= 1 << bit;
		break;
	case avrinfo::PinPurpose::OUTPUT_PIN:
		val.ori |= 1 << bit;
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return val;
}

oriandival CAVRPin::GetDDRVal(bool loaded) const noexcept {
	return loaded
		? GetDDRValImpl(last_loaded_use_, bit_)
		: GetDDRValImpl(use_, bit_);
}

oriandival CAVRPin::GetPORTValImpl(avrinfo::PinPurpose use, int bit, bool inverted) noexcept {
	oriandival val { 0 };
	switch (use) {
	case avrinfo::PinPurpose::INPUTPU_PIN:
		val.ori |= 1 << bit;
		break;
	case avrinfo::PinPurpose::INPUT_PIN:
	case avrinfo::PinPurpose::ANALOG_PIN:
		val.andi |= 1 << bit;
		break;
	case avrinfo::PinPurpose::OUTPUT_PIN:
		if (inverted) val.ori |= 1 << bit;
		else val.andi |= 1 << bit;
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return val;
}

oriandival CAVRPin::GetPORTVal(bool loaded) const noexcept {
	return loaded
		? GetPORTValImpl(last_loaded_use_, bit_, inverted_)
		: GetPORTValImpl(use_, bit_, inverted_);
}

void CAVRPin::UpdateVar(const avrinfo::FirmwareInfo& info, const io_reg_addr& reg) {
	if (status_ == io::Status::relevant) return;
	
	ASSERT_FETCH(var_);
	
	switch (use_) {
	case avrinfo::PinPurpose::INPUTPU_PIN:
	case avrinfo::PinPurpose::INPUT_PIN:
		var_->SetType(npsys::variable::VT_DISCRETE | npsys::variable::IO_SPACE);
		var_->SetBit(bit_);
		var_->SetAddr(reg.pin);
		break;
	case avrinfo::PinPurpose::ANALOG_PIN:
		var_->SetType(npsys::variable::VT_WORD | npsys::variable::IO_SPACE);
		var_->SetAddr(info.rmem.adc_array + bit_ * 2);
		break;
	case avrinfo::PinPurpose::OUTPUT_PIN:
		var_->SetType(npsys::variable::VT_DISCRETE | npsys::variable::IO_SPACE);
		var_->SetBit(bit_);
		var_->SetAddr(reg.port);
		break;
	default:
		ASSERT(FALSE);
		return;
	}

	last_loaded_use_ = use_;
	status_ = io::relevant;

	var_->SetStatus(npsys::variable::Status::good);

	var_.store();
}

bool CAVRPin::IsCanBeUploaded() const noexcept {
	if (status_ == io::assigned || status_ == io::relevant) return true;
	ASSERT_FETCH(var_);
	return var_->RefCount() == 0;
}

int	CAVRPin::GetAllStates() const noexcept  {
	using npsys::CAVRController;
	auto const avr = ctrl.fetch();

	auto const& info = avr->GetPeripheralInfo();
	auto const& pins_cfg = info.pins_cfg;


	if (name_.length() != 3) ASSERT(FALSE);

	int hash = ((int)(name_[2] - '0') << 8) | ((int)name_[1]);

	for (auto const* p = pins_cfg; p->port_letter; p++) {
		if (p->GetHash() == hash) return p->config;
	}
	
	ASSERT(FALSE);
	return -1;
}

int CAVRPin::GetVariableType() const noexcept {
	ASSERT_FETCH(var_);
	return var_->GetType();
}

} // namespace npsys