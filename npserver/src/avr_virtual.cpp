// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "avr_virtual.h"
#include "server.h"
#include <npsys/network.h>
#include <npsys/avr_controller.h>
#include <nplib/utils/config.h>
#include <sim/medium.h>

BOOST_CLASS_EXPORT_GUID(VirtualAvrController, "virtualavrcontroller");
BOOST_CLASS_EXPORT_GUID(VirtualAvrPCLINK, "virtualavrpclink");

void ExecutableVirtualAvrController::run() {
	Medium::get_mutable_instance().AddController(avr_.get());
}

void ExecutableVirtualAvrController::stop() {
	Medium::get_mutable_instance().RemoveController(avr_.get());
}

VirtualAvrController::VirtualAvrController(npsys::controller_n_avr _ctrl)
	: ctrl(_ctrl) {
	switch (ctrl->controller_model) {
	case npsys::hardware::Model::ATMEGA8_VIRTUAL:
		avr_ = std::make_unique<Microcontroller<Atmega8>>(frequency, ctrl->dev_addr);
		avr_->LoadFlash((nplib::config::GetExecutableRootPath() /=
			std::filesystem::path("firmware/atmega8_virtual_" + std::to_string(ctrl->dev_addr) + ".hex")).string());
		break;
	case npsys::hardware::Model::ATMEGA16_VIRTUAL:
		avr_ = std::make_unique<Microcontroller<Atmega8>>(frequency, ctrl->dev_addr);
		avr_->LoadFlash((nplib::config::GetExecutableRootPath() /= 
			std::filesystem::path("firmware/atmega16_virtual_" + std::to_string(ctrl->dev_addr) + ".hex")).string());
		break;
	default:
		throw std::runtime_error("Unknown virtual controller.");
	}
}
// VirtualAvrPCLINK
VirtualAvrPCLINK* VirtualAvrPCLINK::Create() {
	auto vlink = new VirtualAvrPCLINK;
	vlink->avr_ = std::make_unique<Microcontroller<Atmega8>>(frequency, 2);
	try {
		vlink->avr_->LoadFlash((nplib::config::GetExecutableRootPath() /= "firmware/pc-link-virtual.hex").string());
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		std::abort();
	}
	return vlink;
}

int VirtualAvrPCLINK::send_recive(const frame &output, frame &received, size_t expected_length) noexcept {
	using namespace std::chrono;
	const auto& info = avrinfo::AVRInfo::get_instance().GetVirtualPCLinkInfo();

	sram_t& sram = avr_->GetSram();

	uint8_t* buf = &sram[info.data_addr];
	volatile uint8_t& request = sram[info.data_recieved];
	volatile uint8_t& answer = sram[info.data_transmitted];

	if (answer != 0) {
		answer = 0;
		std::cout << "answer != 0" << std::endl;
		return S_COMMUNICATION_ERROR_DEVICE_TIMEOUT;
	}

	auto until = steady_clock::now() + milliseconds(1000);

	memcpy(buf, (const uint8_t*)output, output.length());
	request = 0xFF;

	//	deadline_.expires_from_now(timeout_);
	//	expired_ = false;

	while (until > steady_clock::now()) {
		if (answer && (answer < frame::max_size())) {
			int recv_len = answer;
			received.set_length(recv_len);
			memcpy(received.data(), buf, recv_len);
			answer = 0;
			if (recv_len == expected_length)
			{
				if (_timeout_frame == received) {
					std::cout << "Expected Timeout" << std::endl;
					return 0;
				}
				if (received.is_crc_valid()) {
					return 0;
				} else {
					std::cout << "S_ERROR_CRC_RECIVE" << std::endl;
					return S_COMMUNICATION_ERROR_CRC_RECIVE;
				}
			} else {
				if (_timeout_frame == received) {
					std::cout << "DEVICE DID NOT ANSWER. ADDR: " << (int)output[0] << std::endl;
					return S_COMMUNICATION_ERROR_DEVICE_TIMEOUT;
				} else {
					std::cout << "S_ERROR_UNEXPECTED_FRAMELENGHT:" << recv_len << std::endl;
					std::cout << received << std::endl;
					return S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT;
				}
				return S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT;
			}
		}
	}

	return S_COMMUNICATION_ERROR_DEVICE_TIMEOUT;
}