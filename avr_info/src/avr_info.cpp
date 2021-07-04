// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>
#include <cstring>
#include <assert.h>
#include <iostream>

#include <fstream>
#include <filesystem>
#include <boost/serialization/array.hpp>
#include <boost/archive/binary_iarchive.hpp>

std::filesystem::path avrinfo::AVRInfo::data_path_;

namespace avrinfo {
namespace m8 { extern PeripheralInfo pinfo; }
namespace m16 { extern PeripheralInfo pinfo; }

uint8_t PeripheralInfo::GetIoAddr(const char* name) const {
	for (int i = 0; reg_info[i].addr != 0xff; ++i) {
		if (strcmp(name, reg_info[i].name) == 0)
			return reg_info[i].addr;
	}
	return 0xff;
}

Model string_to_controller_model(const std::string& s) {
	if (s == "atmega8") {
		return Model::ATMEGA8;
	} else if (s == "atmega8_virtual") {
		return Model::ATMEGA8_VIRTUAL;
	} else if (s == "atmega16") {
		return Model::ATMEGA16;
	} else if (s == "atmega16_virtual") {
		return Model::ATMEGA16_VIRTUAL;
	} else {
		assert(false);
		return Model::UNKNOWN_MODEL;
	}
}

const std::string& controller_model_to_string(Model model) noexcept {
	static const std::array<std::string, MODELS_MAX + 1> strs = {
		std::string("ATMEGA8"),
		std::string("VIRTUAL ATMEGA8"),
		std::string("ATMEGA16"),
		std::string("VIRTUAL ATMEGA16"),
		std::string("UNKNOWN MODEL")
	};

	auto index = static_cast<size_t>(model);
	if (index >= strs.size()) return strs[strs.size() - 1];
	
	return strs[index];
}

void AVRInfo::SetDataPath(const std::filesystem::path& path) {
	data_path_ = path;
}

AVRInfo::AVRInfo() {
	if (data_path_.empty()) {
		throw std::runtime_error("AVRInfo data path is not specified");
	}

	pinfo_lst_ = 
	{
		&m8::pinfo,
		&m8::pinfo,
		&m16::pinfo,
		&m16::pinfo,
	};

	try {
		{
			std::vector<std::filesystem::path> files;
			for (const auto& entry : std::filesystem::directory_iterator(data_path_ / "avr" / "ctl_ver")) {
				files.push_back(entry.path());
			}

			std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
				return std::atoi(a.filename().string().c_str()) < std::atoi(b.filename().string().c_str());
				});

			for (auto& path : files) {
				std::ifstream ifs(path, std::ios_base::binary);
				boost::archive::binary_iarchive ia(ifs, boost::archive::no_header | boost::archive::no_tracking);
				avrlst_.push_back({});
				ia >> avrlst_.back();
			}

			{
				std::ifstream ifs(data_path_ / "avr" / "pclink", std::ios_base::binary);
				boost::archive::binary_iarchive ia(ifs, boost::archive::no_header | boost::archive::no_tracking);
				ia >> pclink_info_;
			}

			{
				std::ifstream ifs(data_path_ / "avr" / "pclink_virtual", std::ios_base::binary);
				boost::archive::binary_iarchive ia(ifs, boost::archive::no_header | boost::archive::no_tracking);
				ia >> virtual_pclink_info_;
			}
		}
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		std::abort();
	}
}

const PeripheralInfo& AVRInfo::GetPeripheralInfo(Model model) noexcept {
	return *pinfo_lst_[static_cast<size_t>(model)];
}

const FirmwareInfo& AVRInfo::GetLatestInfo(Model model) noexcept {
	return avrlst_[avrlst_.size() - 1][static_cast<size_t>(model)];
}

const FirmwareInfo& AVRInfo::GetInfo(Model model, int version) {
	if (version >= static_cast<int>(avrlst_.size())) throw std::runtime_error("unknown avr firmware version");
	return avrlst_[version][static_cast<size_t>(model)];
}

} // namespace avrinfo