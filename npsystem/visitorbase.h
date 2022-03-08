// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#define ACCEPT_DECL(BlockClassName) \
	virtual void Accept (BlockClassName* pBlock);

#define ACCEPT_NONE(BlockClassName) \
	virtual void Accept (BlockClassName* pBlock) {}

#define FINDSPECIFICTYPE_IMPL(Visitor, BlockClassName) \
	void Visitor ## ::Accept(BlockClassName* pBlock) { \
		elems_.push_back(pBlock); \
	 } 

template <class Container>
class COneTypeContainer {
protected:
	Container elems_;
public:
	size_t size() const noexcept { return elems_.size(); }
	bool empty() const noexcept { return elems_.empty(); }
	void clear() noexcept { elems_.clear(); }
	auto &operator[] (size_t i) { return elems_[i]; }
	auto begin() noexcept { return elems_.begin(); }
	auto end() noexcept { return elems_.end(); }
	auto begin() const noexcept { return elems_.begin(); }
	auto end() const noexcept { return elems_.end(); }
	auto& get() noexcept { return elems_; }
};

template<typename T>
using COneTypeContainerVector = COneTypeContainer<std::vector<T>>;

template<typename T>
using COneTypeContainerSmallVector32 = COneTypeContainer<boost::container::small_vector<T, 32>>;

template<typename T>
using COneTypeContainerSmallVector64 = COneTypeContainer<boost::container::small_vector<T, 64>>;

template<typename T>
using COneTypeContainerUnorderedSet = COneTypeContainer<std::unordered_set<T>>;

template <class T, class VisitorT>
class CFindType
	: public VisitorT
	, public COneTypeContainerSmallVector32<T*> {
	static_assert(!std::is_abstract<T>::value, "T cannot be abstract.");
public:
	virtual void Accept(T* p) {
		this->elems_.push_back(p);
	}
};

template <class VisitorBase, class... Rest>
void Traversal(typename VisitorBase::InterfacePtr root, Rest&... visitors) {
	ATLASSERT(root != nullptr);
	
	Iter::PreorderIterator<typename VisitorBase::InterfacePtr> it(root);
	for (; !it.IsDone(); it.Next()) {
		((*it)->Visit(visitors), ...);
	}
}


template <class VisitorBase, class... Rest>
void TraversalExclude(typename VisitorBase::InterfacePtr root, typename VisitorBase::InterfacePtr excluded_node_with_children, Rest&... visitors) {
	ATLASSERT(root != nullptr);
	ATLASSERT(excluded_node_with_children != nullptr);

	Iter::PreorderIterator<typename VisitorBase::InterfacePtr> it(root);
	while (it.IsDone() == false) {
		if (*it == excluded_node_with_children) {
			it.NextSkipBranch();
		} else {
			((*it)->Visit(visitors), ...);
			it.Next();
		}
	}
}

#include "blockdecl.h"

class CBlockVisitor {
public:
	typedef CElement* InterfacePtr;

	virtual ~CBlockVisitor() = default;

	ACCEPT_NONE(CFBDBlockComposition)
	ACCEPT_NONE(CSFCBlockComposition)
	ACCEPT_NONE(CBlockCompositionWrapper)
	// slots
	ACCEPT_NONE(CInputSlot)
	ACCEPT_NONE(COutputSlot)
	ACCEPT_NONE(CSlotGroup)
	ACCEPT_NONE(CSFCInputSlot)
	ACCEPT_NONE(CSFCOutputSlot)
	// lines
	ACCEPT_NONE(CLine)
	ACCEPT_NONE(CSFCLine)
	ACCEPT_NONE(CSFCBusIn)
	ACCEPT_NONE(CSFCBusOut)
	// blocks
	#define AAF(x) ACCEPT_NONE(x)
		NPSYSTEM_BLOCKS()
		NPSYSTEM_BLOCKS_SFC()
	#undef AAF
	// Slot types
	ACCEPT_NONE(CBlockInput)
	ACCEPT_NONE(CValue)
	ACCEPT_NONE(CValueRef)
	ACCEPT_NONE(CBitRef)
	ACCEPT_NONE(CInternalRef)
	ACCEPT_NONE(CExternalReference)
	ACCEPT_NONE(CAvrInternalPin)
	ACCEPT_NONE(CModuleValue)
	ACCEPT_NONE(CTextElement)
	ACCEPT_NONE(CStaticTextElement)
	ACCEPT_NONE(CDxEdit)
	ACCEPT_NONE(CInplaceBlockProperty)
	ACCEPT_NONE(CFixedValue)
	ACCEPT_NONE(CScheduleSlider)
	ACCEPT_NONE(CSliderThing)
	ACCEPT_NONE(CSliderTimeChart)
	ACCEPT_NONE(CSizeElement)
};

class IRecreatable {
public:
	virtual void Recreate() = 0;
	virtual CElement* GetBase() = 0;
};

class IConnectableToLine {
public:
	virtual void CollectAffectedGeometries(std::unordered_set<IRecreatable*>&  lines) = 0;
};