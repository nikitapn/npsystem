// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef AVR_INFO_H_
#define AVR_INFO_H_

#include <stdint.h>
#include <filesystem>
#include <string>
#include <vector>
#include <array>
#include <nplib/utils/singleton.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

namespace avrinfo {

enum class Model : int {
	ATMEGA8,
	ATMEGA8_VIRTUAL,
	ATMEGA16,
	ATMEGA16_VIRTUAL,
	UNKNOWN_MODEL
};

constexpr auto MODELS_MAX = static_cast<size_t>(Model::UNKNOWN_MODEL);

enum PinPurpose {
	INPUTPU_PIN = 1, // never changes
	INPUT_PIN = 2,
	OUTPUT_PIN = 4,
	ANALOG_PIN = 8,
	UNKNOWN_PIN = 128
};

struct ControllerPinsConfig {
	const char port_letter;
	const unsigned char pin_n;
	const int config;
	int GetHash() const { return ((int)pin_n << 8) | ((int)port_letter); }
};

struct RegInfo {
	const char name[10];
	const uint8_t addr;
};

struct FirmwareInfo {
	struct rmem_t {
		uint16_t ram_end;
		uint16_t noinit_start;
		uint16_t user_start;
		uint16_t user_end;
		uint16_t info;
		uint16_t tt;
		uint16_t rd;
		uint16_t twi;
		uint16_t adc_array;
		uint16_t eeprcfg;
		uint16_t lt;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & ram_end;
			ar & noinit_start;
			ar & user_start;
			ar & user_end;
			ar & info;
			ar & tt;
			ar & rd;
			ar & twi;
			ar & adc_array;
			ar & eeprcfg;
			ar & lt;
		}
	};

	struct emem_t {
		uint16_t eeprcfg;
		uint16_t org;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & eeprcfg;
			ar & org;
		}
	};

	struct fmem_t {
		uint16_t tt;
		uint16_t rd;
		uint16_t io;
		uint16_t twi;
		uint16_t user_start;
		uint16_t user_end;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & tt;
			ar & rd;
			ar & io;
			ar & twi;
			ar & user_start;
			ar & user_end;
		}
	};

	struct pmem_t {
		uint16_t pagesize;
		uint16_t max_pages;
		uint16_t tt;
		uint16_t rd;
		uint16_t io;
		uint16_t twi;
		uint16_t user_start;
		uint16_t user_end;
		uint16_t max_user_pages;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & pagesize;
			ar & max_pages;
			ar & tt;
			ar & rd;
			ar & io;
			ar & twi;
			ar & user_start;
			ar & user_end;
			ar & max_user_pages;
		}
	};

	struct mccfg_t {
		char partno[20];
		int adc_max;
		int rdata_max;
		int task_max;
		float one_tick_time;
		uint8_t twi_usefull_buffer_max;
		uint8_t twi_requests_max;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & partno;
			ar & adc_max;
			ar & rdata_max;
			ar & task_max;
			ar & one_tick_time;
			ar & twi_usefull_buffer_max;
			ar & twi_requests_max;
		}
	};

	int version;
	rmem_t rmem;
	emem_t emem;
	fmem_t fmem;
	pmem_t pmem;
	mccfg_t mccfg;

	template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & version;
			ar & rmem;
			ar & emem;
			ar & fmem;
			ar & pmem;
			ar & mccfg;
		}
};

struct PCLinkInfo {
	template<typename Archive>
		void serialize(Archive& ar, const int file_version) {	
		}
};

struct VirtualPCLinkInfo {
		uint16_t data_addr;
		uint16_t data_recieved;
		uint16_t data_transmitted;
		uint16_t busy;

		template<typename Archive>
		void serialize(Archive& ar, const int file_version) {
			ar & data_addr;
			ar & data_recieved;
			ar & data_transmitted;
			ar & busy;
		}
};

struct PeripheralInfo {
	const RegInfo* reg_info;
	const char* port;
	const ControllerPinsConfig* pins_cfg;

	uint8_t GetIoAddr(const char* name) const;
};


using avr_controller_info_lst_t = std::array<FirmwareInfo, MODELS_MAX>;

class AVRInfo : public nplib::singleton<AVRInfo> {
	std::array<PeripheralInfo*, MODELS_MAX> pinfo_lst_;
	std::vector<avr_controller_info_lst_t> avrlst_;
	PCLinkInfo pclink_info_;
	VirtualPCLinkInfo virtual_pclink_info_;
	static std::filesystem::path data_path_;
public:
	AVRInfo();

	const PeripheralInfo& GetPeripheralInfo(Model model) noexcept;
	const FirmwareInfo& GetLatestInfo(Model model) noexcept;
	const FirmwareInfo& GetInfo(Model model, int version);
	const PCLinkInfo& GetPCLinkInfo() const noexcept { return pclink_info_; }
	const VirtualPCLinkInfo& GetVirtualPCLinkInfo() const noexcept { return virtual_pclink_info_; }

	static void SetDataPath(const std::filesystem::path& path);
};

Model string_to_controller_model(const std::string& s);
const std::string& controller_model_to_string(Model model) noexcept;

}// namespace avrinfo

#endif // AVR_INFO_H_