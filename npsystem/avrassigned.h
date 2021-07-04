// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "assignedalgorithm.h"
#include "library.h"
#include "hash.h"
#include "libfunction.h"

class CAvrInternalPin;

namespace npsys {

class CAVRFlashObject {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & start_;
		ar & end_;
		ar & size_b_;
		ar & hash_;
	}
protected:
	int start_ = 0;
	int end_ = 0;
	int size_b_ = 0;
	HASHMD5 hash_;
public:
	int AllocSizeP() const noexcept { return end_ - start_ + 1; }
	int Start() const noexcept { return start_; }
	int End() const noexcept { return end_; }
	void SetHash(const HASHMD5& hash) noexcept { hash_ = hash; }
	void SetRange(int begin, int size_b, int pagesize);
	void Reset() noexcept { start_ = end_ = size_b_ = 0; }
	const HASHMD5& GetHash() const noexcept { 
		return hash_; 
	}
	bool IsLoaded() const noexcept { return static_cast<bool>(size_b_); }
};

class CAVRAssignedObjectFile : 
	public odb::common_interface<odb::no_interface>, 
	public CAVRFlashObject 
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CAVRFlashObject>(*this);
		ar & desc.lib;
		ar & desc.obj;
		ar & avr;
		ar & expf;
		ar & status;
	}
public:
	CAVRAssignedObjectFile() = default;
	CAVRAssignedObjectFile(npsys::controller_n_avr& _avr, const LibObjectDesc& _desc) 
		: avr(_avr)
		, desc(_desc)
		, status(io::Status::assigned)
	{
	}

	bool SetFunaddr(const std::string& fun, int addr);
	bool CheckFunctions(std::string& error);
	std::string DefSym();
	//
	LibObjectDesc desc;
	odb::weak_node<npsys::controller_n_avr> avr;
	io::Status status;
	std::vector<LibFunction> expf;
};

class CAVRAssignedAlgorithm : 
	public CAssignedAlgorithm, 
	public CAVRFlashObject 
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CAssignedAlgorithm>(*this);
		ar & boost::serialization::base_object<CAVRFlashObject>(*this);
	}
public:
	CAVRAssignedAlgorithm() = default;
	CAVRAssignedAlgorithm(algorithm_n& _alg, device_n& _avr)
		: CAssignedAlgorithm(_alg, _avr) 
	{
	}
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept {
		return odb::MakeMemento(*this, remote_var_released_, reference_released_, start_, end_, size_b_, hash_);
	}
};

} // namespace npsys