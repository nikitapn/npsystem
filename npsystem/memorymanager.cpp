// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "memorymanager.h"
#include "vartype.h"
#include "network_ext.h"
#include "avrassigned.h"
#include  <avr_info/avr_info.h>

CAvrMemoryManager::CAvrMemoryManager(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only)
	: CMemoryManager(dev, alg)
	, sram_only_(sram_only) {
	
	auto avr = dev.cast<npsys::controller_n_avr>();

	auto const& info = avr->GetFirmwareInfo();
	
	SetRam(info.rmem.user_start, info.rmem.user_end, info.rmem.ram_end + 1);

	if (sram_only) return;

	page_size_ = info.pmem.pagesize;
	user_start_ = info.pmem.user_start;
	user_end_ = info.pmem.user_end;

	flash_ = std::make_unique<uint8_t[]>(info.pmem.max_pages);
	memset(flash_.get(), 0x00, info.pmem.max_pages);

	avr->assigned_algs.fetch_all_nodes();
	avr->assigned_ofs.fetch_all_nodes();

	for (auto& alg : avr->assigned_algs) {
		auto obj = static_cast<npsys::CAVRAssignedAlgorithm*>(alg.get());
		if (obj->IsLoaded()) {
			memset(flash_.get() + obj->Start(), 0xFF, obj->AllocSizeP());
		}
	}

	for (auto& obj : avr->assigned_ofs) {
		if (obj->IsLoaded()) {
			memset(flash_.get() + obj->Start(), 0xFF, obj->AllocSizeP());
		}
	}
}

int CAvrMemoryManager::AllocatePageMemory(int objectSize) {
	ASSERT(sram_only_ == false);

	int page_count = (objectSize % page_size_) ?
		objectSize / page_size_ + 1 :
		objectSize / page_size_;

	for (int k = 0, i = user_start_; i <= user_end_; ++i) {
		if (flash_[i] == 0x00) {
			k++;
		} else {
			k = 0;
			continue;
		}
		if (k == page_count) {
			memset(&flash_[i - (page_count - 1)], 0xFF, page_count);
			return i - (page_count - 1);
		}
	}

	throw flash_bad_alloc();
}


std::unique_ptr<CMemoryManager> 
CMemoryManager::Create(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only)
{
	// redo this
	switch (dev.cast<npsys::controller_n>()->controller_model) {
	case npsys::hardware::Model::ATMEGA8:
	case npsys::hardware::Model::ATMEGA8_VIRTUAL:
	case npsys::hardware::Model::ATMEGA16:
	case npsys::hardware::Model::ATMEGA16_VIRTUAL:
		return std::make_unique<CAvrMemoryManager>(dev, alg, sram_only);
	default:
		ASSERT(FALSE);
		return std::unique_ptr<CMemoryManager>();
	}
}

std::unique_ptr<CMemoryManager>
CMemoryManager::CreateGeneric(npsys::device_n dev, bool sram_only) {
	return CMemoryManager::Create(dev, {}, sram_only);
}

std::unique_ptr<CMemoryManager>
CMemoryManager::CreateForAlgorithm(npsys::device_n dev, npsys::fbd_control_unit_n alg, bool sram_only) {
	return CMemoryManager::Create(dev, alg, sram_only);
}

void CMemoryManager::SetRam(int user_ram_begin, int user_ram_end, int ram_size) {
	user_ram_begin_ = user_ram_begin;
	user_ram_end_ = user_ram_end;
	ram_size_ = ram_size;
	
	sram_.reset(new memcell[ram_size_]);
	memset(sram_.get(), 0x00, ram_size_ * sizeof(memcell));
	
	auto fill = [&](const npsys::variable_n& var) -> void {
		if (var->GetStatus() != npsys::variable::Status::not_loaded) {
			if (var->IsBit()) {
				int bit = var->GetBit();;
				sram_[var->GetAddr()].alg_id = var->GetAlg().id();
				sram_[var->GetAddr()].value |= (1 << bit);
				sram_[var->GetAddr()].value |= (2 << bit);
				sram_[var->GetAddr()].bitcell = true;
			} else {
				for (int i = 0; i < var->GetSize() + 1; ++i) {
					sram_[var->GetAddr() + i].alg_id = var->GetAlg().id();
					sram_[var->GetAddr() + i].value = 0xFF;
				}
			}
		}
	};

	dev_->allocated_vars_.fetch_all_nodes();
	dev_->vars_owner_released_.fetch_all_nodes();

	for (const auto& var : dev_->allocated_vars_) fill(var);
	for (const auto& var : dev_->vars_owner_released_) fill(var);
}

void CMemoryManager::AllocateMemory(npsys::variable_n& var_n, int fix_bit_n) {
	ASSERT((!var_n->IsBit()) || (var_n->IsBit() || !var_n->GetAlg().is_invalid_id() && IsAlgorithm()));

	auto var = var_n.get();
	if (var->GetStatus() != npsys::variable::Status::not_loaded) return;
	if (var->IsBit()) {
		for (int i = user_ram_begin_; i <= user_ram_end_; ++i) {
			if (sram_[i].alg_id == var->GetAlg().id() &&
				sram_[i].bitcell &&
				sram_[i].value != 0xFF
				) {
				int c = -1;
				while (sram_[i].value & (1 << ++c));
				if (fix_bit_n != -1 && fix_bit_n != c) continue;
				sram_[i].value |= (3 << c);
				var->SetAddr(i);
				ASSERT(c < 8);
				var->SetBit(c);
				goto l_allocated;
			}
		}
		for (int i = user_ram_begin_; i <= user_ram_end_; ++i) {
			if (!sram_[i].value) {
				sram_[i].alg_id = var->GetAlg().id();
				sram_[i].bitcell = true;
				if (fix_bit_n != -1) {
					sram_[i].value |= (3 << fix_bit_n);
					var->SetBit(fix_bit_n);
				} else {
					sram_[i].value |= (3 << 0);
					var->SetBit(0);
				}
				var->SetAddr(i);
				goto l_allocated;
			}
		}
	} else {
		int addr;
		FindFreeSpace(var->GetAlg(), var->GetSize() + var->IsQuality(),
			user_ram_begin_, user_ram_end_, &addr);
		var->SetAddr(addr);
		goto l_allocated;
	}
	throw sram_bad_alloc(dev_);
l_allocated:
	var->SetStatus(npsys::variable::Status::allocated);
	dev_->allocated_vars_.add_node(var_n);
	allocated_vars_.push_back(var_n);
	last_count_++;
}


void CMemoryManager::AllocateMemory(COneVariable* var) {
	AllocateMemory(var->variable_);
}

void CMemoryManager::AllocateMemory(CVarContainer* var_c) {
	if (var_c->Empty() || 
		var_c->variables_[0]->GetStatus() != npsys::variable::Status::not_loaded) return;
	int addr;
	auto total_size = var_c->VarByteSize();
	FindFreeSpace(var_c->variables_[0]->GetAlg(), total_size, user_ram_begin_, user_ram_end_, &addr);
	for (auto& var : var_c->variables_) {
		var->SetAddr(addr);
		var->SetStatus(npsys::variable::Status::allocated);
		addr += var->GetSize() + var->IsQuality();
		dev_->allocated_vars_.add_node(var);
		allocated_vars_.push_back(var);
		last_count_++;
	}
}

void CMemoryManager::FindFreeSpace(
	const odb::weak_node<npsys::fbd_control_unit_n>& alg,
	int size,
	int memBegin,
	int memEnd,
	int* pAddr)
{
	int k = 0;
	for (int i = memBegin; i < memEnd; ++i) {
		if (sram_[i].value) {
			k = 0;
			continue;
		}
		if (++k == size) {
			int begin = i - (size - 1);
			for (int j = i; j >= begin; --j) {
				sram_[i].alg_id = alg.id();
				sram_[j].value = 0xFF;
			}
			(*pAddr) = begin;
			return;
		}
	}
	throw sram_bad_alloc(dev_);
}
void CMemoryManager::Commit() {
	if (allocated_vars_.empty()) return;
	for (auto var : allocated_vars_) {
		if (var->GetStatus() == npsys::variable::Status::allocated) {
			var->SetStatus(npsys::variable::Status::good);
		} else if (var->GetStatus() == npsys::variable::Status::allocated_from_another_algorithm) {
			var->SetStatus(npsys::variable::Status::good_allocated_from_another_algorithm);
		} else {
			ASSERT(FALSE);
		}
	}
}
void CMemoryManager::SaveState() {
	last_count_ = 0;
}
void CMemoryManager::RollbackState() {
	Rollback(last_count_);
	last_count_ = 0;
}

void CMemoryManager::RollbackAll() {
	Rollback(allocated_vars_.size());
	last_count_ = 0;
}

void CMemoryManager::Rollback(size_t n) {
	while (n) {
		auto var = allocated_vars_.back();
		allocated_vars_.pop_back();

		var->SetStatus(npsys::variable::Status::not_loaded);

		memcell* p = &sram_[var->GetAddr()];

		if (p->bitcell) {
			p->value &= ~(3 << var->GetBit());
			if (!p->value) {
				p->bitcell = false;
			}
		} else {
			for (int i = 0; i < var->GetSize() + 1; ++i) {
				p[i].value = 0x00;
			}
		}
		n--;
	}
}