// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "stuff.h"
#include <avr_info/avr_info.h>
#include <npsys/variable.h>


namespace npsys {
class CAVRController;
};
class COneVariable;
class CVarContainer;

class sram_bad_alloc : public std::runtime_error {
public:
	sram_bad_alloc(npsys::device_n& _dev) 
		: std::runtime_error("sram_bad_alloc")
		, dev(_dev) {}

	npsys::device_n dev;
};

class flash_bad_alloc : public std::runtime_error
{
public:
	flash_bad_alloc() :
		std::runtime_error("flash_bad_alloc") {}
};

class CMemoryManager
{
public:
	CMemoryManager() = delete;
	CMemoryManager(const CMemoryManager&) = delete;
	CMemoryManager(CMemoryManager&&) = default;
	virtual ~CMemoryManager() = default;
	void AllocateMemory(npsys::variable_n& var, int fix_bit_n = -1);
	void AllocateMemory(COneVariable* pVar);
	void AllocateMemory(CVarContainer* pVarContainer);
	void Commit();
	void SaveState();
	void RollbackState();
	void RollbackAll();
	// allocating bit variables is forbidden
	static std::unique_ptr<CMemoryManager> 
		CreateGeneric(npsys::device_n dev, bool sram_only);
	// can be used for allocating bit variables
	static std::unique_ptr<CMemoryManager>
		CreateForAlgorithm(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only);
protected:
	static std::unique_ptr<CMemoryManager>
		Create(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only);
	CMemoryManager(npsys::device_n& dev, npsys::fbd_control_unit_n& alg)
		: dev_(dev)
		, alg_(alg)
		, last_count_(0)
	{
	}
	bool IsAlgorithm() const noexcept { return !alg_.is_invalid_node(); }
	void SetRam(int user_ram_begin, int user_ram_end, int ram_size);
private:
	void FindFreeSpace(
		const odb::weak_node<npsys::fbd_control_unit_n>& alg, 
		int var_size, 
		int memBegin, 
		int memEnd, 
		int* pAddr);
	void Rollback(size_t n);
	struct memcell {
		oid_t alg_id;
		unsigned char value;
		bool bitcell;
	};
	std::unique_ptr<memcell[]> sram_;
	int last_count_;
	int user_ram_begin_;
	int user_ram_end_;
	int ram_size_;
	var_list allocated_vars_;
protected:
	npsys::device_n dev_;
	npsys::fbd_control_unit_n alg_;
};


class CAvrMemoryManager : public CMemoryManager {
public:
	CAvrMemoryManager(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only);
	int AllocatePageMemory(int objectSize);
private:
	std::unique_ptr<uint8_t[]> flash_;
	bool sram_only_;
	int page_size_;
	int user_start_;
	int user_end_;
};