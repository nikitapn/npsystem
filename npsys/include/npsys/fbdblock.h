// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"
#include <vector>

#if defined(_CONFIGURATOR_) || defined(_NPSYS_) || defined(_NPSERVER_) || defined(_NPWEBSERVER_)
#include "other/online_value.h"
#endif

#ifdef _CONFIGURATOR_
class CTreeBlock;
class CSlot;
class CInputSlot;
class COutputSlot;
class CBlock;
class CDlg_Parameter;
#endif

namespace npsys {
class CFBDSlot
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<CFBDSlot>
	, public odb::modified_flag
	, public odb::self_node_member<npsys::fbd_slot_n> {
#ifdef _CONFIGURATOR_
	friend CDlg_Parameter;
#endif
	friend boost::serialization::access;
/*
#ifdef _CONFIGURATOR_
	template<class Archive>
	void save(Archive & ar, const unsigned int) const {}
	template<class Archive>
	void load(Archive & ar, const unsigned int) {
		e_slot->top = this;
		e_slot->GetTextElement()->SetText(name_);
		e_slot->AlignText();
	}
#endif
*/
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name_;
		serialize_if_not_special(block_parent, ar, {});
		if (file_version > 1) {
			serialize_if_not_special(var, ar, {});
			ar & flags_;
		}
		if (file_version >= 3) ar & strings;
		if (file_version >= 4) ar & fbd_unit;
		//
#ifdef _CONFIGURATOR_
		serialize2(ar, file_version);
		//ar & e_slot;
		//boost::serialization::split_member(ar, *this, file_version);
#endif
	}
	template<class Archive>
	void serialize2(Archive& ar, const unsigned int file_version);
public:
	enum : uint32_t {
		F_HISTORY = 1
	};

	CFBDSlot() = default;
	
	template<typename T>
	CFBDSlot(T&& name, npsys::fbd_block_n& _block_parent)
		: odb::systree_item<CFBDSlot>(std::forward<T>(name))
		, block_parent(_block_parent)
		, flags_(0)
	{
	}

	bool IsHistoryFlag() const noexcept {
		return flags_ & F_HISTORY;
	}

	std::string path() const noexcept;
	static fbd_slot_n get_by_path(std::string_view path);
	static std::vector<std::string_view> split_path(std::string_view str);

	// public members
	variable_n var;
	odb::weak_node<fbd_block_n> block_parent;
#if defined(_CONFIGURATOR_) || defined(_NPSYS_) || defined(_NPSERVER_) || defined(_NPWEBSERVER_)
	online_value online;
#endif
	odb::weak_node<strings_n> strings;
	odb::weak_node<fbd_control_unit_n> fbd_unit;
#ifdef _CONFIGURATOR_
	std::unique_ptr<CSlot> e_slot;
#endif
private:
	uint32_t flags_ = 0;
};

class SlotList {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & i_size_;
		ar & o_size_;
		ar & slots_;
	}

	size_t i_size_ = 0;
	size_t o_size_ = 0;
	odb::inplace_node_list<fbd_slot_n> slots_;
public:
	using iterator = odb::inplace_node_list<fbd_slot_n>::iterator;

	auto begin() noexcept { return slots_.begin(); }
	auto end() noexcept { return slots_.end(); }
	auto begin_in() noexcept { return slots_.begin(); }
	auto end_in() noexcept {
		auto it = slots_.begin();
		std::advance(it, i_size_);
		return it;
	}
	auto begin_out() noexcept { return end_in(); }
	auto end_out() noexcept { return end(); }
	// reverse
	auto rbegin() noexcept { return slots_.rbegin(); }
	auto rend() noexcept { return slots_.rend(); }
	auto rbegin_out() noexcept { return slots_.rbegin(); }
	auto rend_out() noexcept {
		auto it = slots_.rbegin();
		std::advance(it, o_size_);
		return it;
	}
	auto rbegin_in() noexcept { return rbegin_out(); }
	auto rend_in() noexcept { return slots_.rend(); }
	void add_input_slot(fbd_slot_n& slot) {
		auto it = end_in();
		slots_.insert(it, slot);
		i_size_++;
	}
	void add_output_slot(fbd_slot_n& slot) {
		auto it = end_out();
		slots_.insert(it, slot);
		o_size_++;
	}
	void fetch_all_nodes() noexcept {
		slots_.fetch_nodes();
	}
	size_t i_size() const noexcept {
		return i_size_;
	}
	size_t o_size() const noexcept {
		return o_size_;
	}

	void clear_with_nodes() noexcept {
		for (auto& slot : slots_) slot.remove();
		slots_.clear();
		i_size_ = o_size_ = 0;
	}
	
#ifdef _CONFIGURATOR_
	 iterator erase(fbd_slot_n& slot) {
		auto it = slots_.find(slot);
		assert(it != slots_.end());

		if (it >= begin_in() && it < begin_out()) i_size_--;
		else o_size_--;

		return slots_.erase(it);
	}
	CSlot& at_i(size_t ix) noexcept {
		assert(ix < i_size_);
		auto it = begin_in();
		std::advance(it, ix);
		return *((*it)->e_slot.get());
	}
	CSlot& at_o(size_t ix) noexcept {
		assert(ix < o_size_);
		auto it = begin_out();
		std::advance(it, ix);
		return *((*it)->e_slot.get());
	}
	CSlot* GetSlotByName(std::string_view slot_name) noexcept {
		auto founded = std::find_if(begin(), end(), [=](auto& slot) { return slot->get_name() == slot_name; });
		return founded != end() ? (*founded)->e_slot.get() : nullptr;
	}
#endif
};

class CFBDBlock 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<CFBDBlock>
	, public odb::modified_flag
	, public odb::self_node_member<npsys::fbd_block_n> 
{
	friend boost::serialization::access;
/*
#ifdef _CONFIGURATOR_
	template<class Archive>
	void save(Archive & ar, const unsigned int) const {}
	template<class Archive>
	void load(Archive & ar, const unsigned int) {
		e_block->top = this;
		slots.fetch_all_nodes();
		for (auto& slot : slots) {
			slot->e_slot->SetParent(e_block.get());
			slot->e_slot->CreateRgnShape();
			slot->e_slot->UpdateTransformedGeometry();
			e_block->elements_.push_back(slot->e_slot.get());
		}
	}
#endif
*/
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		(void)file_version;
		ar & name_;
		ar & slots;
#ifdef _CONFIGURATOR_
		serialize2(ar, file_version);
//		ar & e_block;
//		boost::serialization::split_member(ar, *this, file_version);
#endif
	}
	template<class Archive>
	void serialize2(Archive& ar, const unsigned int file_version);
public:
	CFBDBlock() = default;
	template<typename T>
	CFBDBlock(T&& name) : odb::systree_item<CFBDBlock>(std::forward<T>(name)) {}
	//
	SlotList slots;

#ifdef _CONFIGURATOR_
	CTreeBlock* tree_block = nullptr;
	std::unique_ptr<CBlock> e_block;
#endif // _CONFIGURATOR_
};


inline std::string get_slot_full_name(CFBDSlot* slot) noexcept {
	auto block = slot->block_parent.fetch();
	return block->get_name() + '/' + slot->get_name();
}

} // namespace npsys

BOOST_CLASS_VERSION(npsys::CFBDSlot, 4);
