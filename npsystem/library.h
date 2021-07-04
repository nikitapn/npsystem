// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

namespace npsys {

enum class LIBRARY_TARGET {
	LIBRARY_TARGET_AVR,
};

enum class FILE_TYPE {
	FILE_TYPE_ASM,
	FILE_TYPE_C,
	FILE_TYPE_TXT,
	FILE_TYPE_HEADER,
	FILE_TYPE_UNK
};

class CLibraryObjectFile {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name;
		ar & expf;
	}
public:
	std::string name;
	std::vector<std::string> expf;
};

using DepTable = std::unordered_map<std::string, std::vector<std::string>>;
using ObjectList = std::unordered_map<std::string, CLibraryObjectFile>;
using AdditionalLibs = std::unordered_set<std::string>;

class CLibrary {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name;
		ar & libobjects;
		ar & additionalLibs;
		ar & table;
	}
public:
	std::string name;
	ObjectList libobjects;
	AdditionalLibs additionalLibs;
	DepTable table;
};

class LibList {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & data_;
	}
public:
	const npsys::CLibrary& get_by_name(const std::string& libname) const {
		auto it = data_.find(libname);
		if (it == data_.end()) throw std::runtime_error("library \""+ libname +"\" does not exist");
		return it->second;
	}
	LibList& operator=(const std::vector<CLibrary>& src) {
		data_.clear();
		for (const auto& l : src)
			data_[l.name] = l;
		return *this;
	}
private:
	std::unordered_map<std::string, CLibrary> data_;
};

struct LibObjectDesc {
	std::string lib;
	std::string obj;
};

inline bool operator == (const LibObjectDesc& ob_l, const LibObjectDesc& ob_r) {
	return ((ob_l.lib == ob_r.lib)
		&& (ob_l.obj == ob_r.obj));
}

inline bool operator < (const LibObjectDesc& ob_l, const LibObjectDesc& ob_r) {
	return ((ob_l.lib < ob_r.lib)
		&& (ob_l.obj < ob_r.obj));
}


} // namespace npsys

namespace std {
template<>
struct hash<npsys::LibObjectDesc> {
	std::size_t operator()(npsys::LibObjectDesc const & s) const {
		std::size_t val = 0;
		boost::hash_combine(val, s.lib);
		boost::hash_combine(val, s.obj);
		return val;
	}
};
} // namespace std

namespace npsys {
using ObjectDescList = std::unordered_set<npsys::LibObjectDesc>;
extern LibList libs;
};