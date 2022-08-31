// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "element.h"

class CSlot;

class CBlockCompositionBase 
	: public CElement
{
private:
	template<class Archive>
	void save(Archive & ar, const unsigned int /*file_version*/) const {
		size_t size = 0;
		for (auto& e : elements_) {
			if (e->IsSerializedByComposition()) size++;
		}
		ar << size;
		for (auto& e : elements_) {
			if (e->IsSerializedByComposition()) ar << e;
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
protected:
	std::vector<CElement*> elements_;

	template<typename TLineContainer, typename TSlotContainer>
	void ConnectLinesToSlotsAfterDeserialization(TLineContainer& lines, TSlotContainer& slots) {
		//static_assert(std::is_pointer_v<TLineContainer::value_type>);
		//static_assert(std::is_pointer_v<TSlotContainer::value_type>);

		typename TSlotContainer::value_type founded[2];
		for (auto l : lines) {
			size_t ix = 0;
			for (auto s : slots) {
				int slot_id = s->GetId();
				if (l->GetInID() == slot_id ||
					l->GetOutID() == slot_id) {
					founded[ix++] = s;
					if (ix == 2) break;
				}
			}
			if (ix < 2) {
				RemoveElement(l);
				delete l;
			} else {
				l->SetSlot(founded[0], false);
				l->SetSlot(founded[1], false);
				l->ConnectSlots(false, this);
			}
		}
	}
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
	void FBD_RestorePointers();
	void SFC_RestorePointers();
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

inline void RemoveFromQuadrant(CBlockCompositionBase* pComposition, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pComposition);
	for (; !it.IsDone(); it.Next())
		qtRoot->DeleteElement(*it);
}

inline void PasteToQuadrant(CBlockCompositionBase* pComposition, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pComposition);
	for (; !it.IsDone(); it.Next())
		qtRoot->AddElement(*it);
}

class CBlockComposition : public CBlockCompositionBase
{
	friend class PasteClipboardCommand;

	int	cc_ = -1;
	std::vector<std::unique_ptr<Command>> commands_;
public:
	bool CheckName(const std::string& name, CElement* elem);
	void AddCommand(std::unique_ptr<Command>&& cmd, bool bExecute);
	void Redo();
	int Undo();
	void PermanentExecuteCommands() noexcept;
	bool IsRedoCommands();
	bool IsUndoCommands();
	void SetSelect(bool select) noexcept;

	void DeleteSelectedElements(MyQuadrantNode* qtRoot);
};