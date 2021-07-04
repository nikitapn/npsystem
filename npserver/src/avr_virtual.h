// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "bridge.h"
#include <avr_info/avr_info.h>
#include <sim/sim.h>
#include <npdb/db.h>
#include <npsys/header.h>

class VirtualDevice 
	: public odb::common_interface<VirtualDevice>
	/*, public bridge*/ 
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int /*file_version*/) {}
public:
	virtual oid_t id() = 0;
	virtual void run() = 0;
	virtual void stop() = 0;
};

class VirtualController 
	: public VirtualDevice 
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<VirtualDevice>(*this);
	}
};

class ExecutableVirtualAvrController 
	: public VirtualController 
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<VirtualController>(*this);
		ar & avr_;
	}
public:
	std::unique_ptr<AvrMicrocontroller> avr_;
	virtual void run() override final;
	virtual void stop() override final;
	virtual ~ExecutableVirtualAvrController() = default;
protected:
	static constexpr uint64_t frequency = 16'000'000ULL;
};

class VirtualAvrPCLINK 
	: public bridge
	, public ExecutableVirtualAvrController
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<ExecutableVirtualAvrController>(*this);
	}
public:
	VirtualAvrPCLINK() = default;
	virtual int send_recive(const frame &output, frame &recived, size_t expected_length) noexcept override;
	virtual oid_t id() { return npsys::VIRTUAL_PC_LINK_ID; }
	static VirtualAvrPCLINK* Create();
};

class VirtualAvrController 
	: public ExecutableVirtualAvrController 
{
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		ctrl.fetch();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<ExecutableVirtualAvrController>(*this);
		ar & ctrl;
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	npsys::controller_n_avr ctrl;
	VirtualAvrController() = default;
	VirtualAvrController(npsys::controller_n_avr _ctrl);
	virtual oid_t id() { return ctrl.id(); }
};


