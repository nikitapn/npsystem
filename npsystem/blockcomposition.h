#pragma once

#include <npsys/header.h>
#include <npsys/fbdblock.h>
#include "block.h"


// class CSlot;
// class CBlock;

class CBlockCompositionBase 
	: public CElement
{
private:
	template<class Archive>
	void save(Archive & ar, const unsigned int /*file_version*/) const {
		size_t size = 0;
		for (auto& e : elements_) {
			if (!e->IsNode()) size++;
		}
		ar << size;
		for (auto& e : elements_) {
			if (!e->IsNode()) ar << e;
		}
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int /*file_version*/) {
		size_t size;
		ar >> size;
		while (size--) {
			CElement* e;
			ar >> e;
			elements_.push_back(e);
		}
	}
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		try {
			ar & boost::serialization::base_object<CElement>(*this);
			boost::serialization::split_member(ar, *this, file_version);
		} catch (std::exception& e) {
			MessageBoxA(0, e.what(), 0, 0);
		}
	}
	Iter::StlContainerIterator<CElement*, std::vector> m_iterator;
public:
	using Container = std::vector<CElement*>;
	using ElementIterator = Container::iterator;
	using ConstElementIterator = Container::const_iterator;

	bool Empty(void) { return elements_.empty(); }
	void RemoveElement(CElement* pElement);
	virtual CManipulator* GetManipulator() final { return nullptr; }
	virtual	_Iterator* CreateIterator();
	void Push(CElement* element);
	size_t Push(CBlockCompositionBase* other);
	void Pop(void);
	void Pop(size_t n);
	
	virtual void Draw(CGraphics* pGraphics) final {}
	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags) final { return false; }
	virtual void Move(float x, float y) final;
	virtual void MoveDxDy(float dx, float dy) final;
	
	void UpdateAllBezierLines();
	void ClearVariables();
	static std::string CreateUniqueName(ConstElementIterator begin, ConstElementIterator end, const std::string& pattern);
	std::string CreateUniqueName(const std::string& pattern) const;
	virtual ElementIterator begin() { return elements_.begin(); }
	virtual ElementIterator end() { return elements_.end(); }
protected:
	void RestorePointers();
protected:
	std::vector<CElement*> elements_;
};

inline void CBlockCompositionBase::RemoveElement(CElement* pElement) {
	bool ok = VectorFastErase(elements_, pElement);
	ASSERT(ok == true);
}
inline void	CBlockCompositionBase::Push(CElement* pNew) {
	elements_.push_back(pNew);
}
inline void CBlockCompositionBase::Pop(void) {
	elements_.pop_back();
}
inline void CBlockCompositionBase::Pop(size_t n) {
	ASSERT(n <= elements_.size());
	for (; n; --n) elements_.pop_back();
}
inline size_t CBlockCompositionBase::Push(CBlockCompositionBase* other) {
	std::copy(other->elements_.begin(), other->elements_.end(), std::back_inserter(elements_));
	return other->elements_.size();
}

class npsys::CAlgorithmExt;

class Command;

class CBlockComposition 
	: public CBlockCompositionBase
	, public odb::common_interface<odb::no_interface>
{
	friend class PasteClipboardCommand;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CBlockCompositionBase>(*this);
		ar & block_factory_;
	}
public:
	DECLARE_TYPE_NAME("EDITOR")
	DECLARE_VISITOR()
	CBlockComposition() 
		: block_factory_(this) 
	{
	}
	~CBlockComposition();
	CSlot* GetSlotById(int id);
	void DeleteSelectedElements(MyQuadrantNode* qtRoot);
	bool CheckName(const std::string& name, CElement* elem);
	void AddCommand(Command* cmd, bool bExecute);
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
	void Redo();
	int Undo();
	void PermanentExecuteCommands() noexcept;
	bool IsRedoCommands();
	bool IsUndoCommands();
	void SetSelect(bool select) noexcept;
	void SetAlgorithm(
		npsys::CAlgorithmExt* alg, 
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
		RestorePointers();
	}
	npsys::CAlgorithmExt* GetAlgorithm() noexcept {
		ATLASSUME(alg_ != nullptr);
		return alg_;
	}
	CBlockFactory* GetBlockFactory() { return &block_factory_; }
private:
	odb::node_list<npsys::fbd_block_n> blocks_;
	CBlockFactory block_factory_;
	int	cc_ = -1;
	std::vector<std::unique_ptr<Command, cpp_command_delete>> commands_;
	npsys::CAlgorithmExt* alg_ = nullptr;
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
		RestorePointers();
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CBlockCompositionBase>(*this);
		ar & blocks_;
		ar & alg_;
		boost::serialization::split_member(ar, *this, file_version);
	}
	
	void ReplaceElement(CElement* old_element, npsys::fbd_block_n& new_block);

public:
	DECLARE_TYPE_NAME("WRAPPER")
	DECLARE_VISITOR()
	CBlockCompositionWrapper() = default;
	CBlockCompositionWrapper(npsys::algorithm_n& alg) 
		: alg_(alg) {}
	
	npsys::algorithm_n alg_;
	std::vector<npsys::fbd_block_n> blocks_;
	virtual ~CBlockCompositionWrapper() { 
		elements_.clear(); // prevent deletion
	}
	bool Empty(void) { return blocks_.empty() && elements_.empty(); }
	void Adapt(CBlockComposition* dest);
	void DeleteElements();
	void Push(CElement* element) noexcept;
	void PushBlock(npsys::CFBDBlock* block);
};
