#pragma once

#include "blockcomposition.hpp"
#include "fbd_block_factory.hpp"

#include "block.h"
#include <npsys/fbdblock.h>

class CFBDBlockComposition 
	: public CBlockComposition
	, public odb::common_interface<odb::no_interface>
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CBlockCompositionBase>(*this);
		ar & block_factory_;
	}

	odb::node_list<npsys::fbd_block_n> blocks_;
	CFBDBlockFactory block_factory_;
	npsys::CFBDControlUnit* alg_ = nullptr;
public:
	DECLARE_TYPE_NAME("FBD_EDITOR")
	DECLARE_VISITOR()
	
	auto& blocks() { return blocks_; }

	void AddBlock(npsys::fbd_block_n& block) {
		blocks_.add_node(block);
		Push(block->e_block.get());
		block->e_block->OnBlockAddedToEditor();
	}
	
	void RemoveBlock(npsys::fbd_block_n& block) {
		blocks_.fast_erase(block);
		RemoveElement(block->e_block.get());
		block->e_block->OnBlockDeletedFromEditor();
	}

	void PopBlock() {
		blocks_.pop_back();
		Pop();
	}
	
	void SetAlgorithm(
		npsys::CFBDControlUnit* alg, 
		odb::node_list<npsys::fbd_block_n>& blocks) noexcept 
	{
		ATLASSERT(alg);
		ATLASSUME(!alg_);
		ATLASSUME(blocks.loaded());
		alg_ = alg;
		blocks_ = blocks;
		for (auto& block : blocks_) {
			Push(block->e_block.get());
		}
		FBD_RestorePointers();
	}

	npsys::CFBDControlUnit* GetAlgorithm() noexcept {
		ATLASSUME(alg_ != nullptr);
		return alg_;
	}

	CSlot* GetSlotById(int id);

	CFBDBlockFactory* GetBlockFactory() { return &block_factory_; }
	
	virtual ~CFBDBlockComposition();
	
	CFBDBlockComposition() 
		: block_factory_(this) 
	{
	}
};

class CBlockCompositionWrapper : public CBlockCompositionBase {
	friend class CBlockComposition;
	friend class PasteClipboardCommand;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int) const {}
	template<class Archive>
	void load(Archive & ar, const unsigned int) {
		for (auto& block : blocks_) {
			elements_.push_back(block->e_block.get());
		}
		FBD_RestorePointers();
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CBlockCompositionBase>(*this);
		ar & blocks_;
		ar & fbd_unit_;
		boost::serialization::split_member(ar, *this, file_version);
	}
	
	void ReplaceElement(CElement* old_element, npsys::fbd_block_n& new_block);

public:
	npsys::fbd_control_unit_n fbd_unit_;
	std::vector<npsys::fbd_block_n> blocks_;

	DECLARE_TYPE_NAME("WRAPPER")
	DECLARE_VISITOR()

	bool Empty(void) { return blocks_.empty() && elements_.empty(); }
	void Adapt(CFBDBlockComposition* dest);
	void DeleteElements();
	void Push(CElement* element) noexcept;
	void PushBlock(npsys::CFBDBlock* block);

	virtual ~CBlockCompositionWrapper() { 
		elements_.clear(); // prevent deletion
	}

	CBlockCompositionWrapper() = default;
	CBlockCompositionWrapper(npsys::fbd_control_unit_n fbd_unit) 
		: fbd_unit_(fbd_unit) 
	{
	}
};
