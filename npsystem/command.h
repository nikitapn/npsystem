// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "stuff.h"
#include "element.h"
#include "blockcomposition.h"
#include "block.h"

class CBlockComposition;
class CBlock;
class CGroup;
class CParameter;
class CSlot;
class CSlotType;
class CBlockCompositionWrapper;


inline void RemoveFromQuadrant(CElement* pElem, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pElem);
	for (; !it.IsDone(); it.Next()) {
		qtRoot->DeleteElement(*it);
	}
	qtRoot->DeleteElement(pElem);
}

inline void PasteToQuadrant(CElement* pElem, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pElem);
	for (; !it.IsDone(); it.Next()) {
		qtRoot->AddElement(*it);
	}
	qtRoot->AddElement(pElem);
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

class Command {
protected:
	bool executed_ = false;
public:
	virtual ~Command() = default;
	virtual void Execute()  = 0;
	virtual void UnExecute() = 0;
	virtual void ExecutePermanent() { };
	bool IsExecuted() const noexcept { return executed_; }
};

class DeleteElementFromGroupSimple : public Command {
	CElement* element_;
	CGroup* group_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	DeleteElementFromGroupSimple(
		CElement* element,
		CGroup* group, 
		CElement::MyQuadrantNode* qtRoot);
	virtual void Execute();
	virtual void UnExecute();
	virtual void ExecutePermanent();
};

class InsertElementToGroupSimple : public DeleteElementFromGroupSimple {
public:
	InsertElementToGroupSimple(
		CElement* element,
		CGroup* group,
		CElement::MyQuadrantNode* qtRoot)
		: DeleteElementFromGroupSimple(element, group, qtRoot)
	{
	}
	virtual void Execute() { DeleteElementFromGroupSimple::UnExecute(); }
	virtual void UnExecute() { DeleteElementFromGroupSimple::Execute(); }
	virtual void ExecutePermanent() {}
};

/*
typedef void Action();
template <class Receiver>
class SimpleCommand : public Command {
	Action _action;
	Receiver* _receiver;
public:
	SimpleCommand(Receiver*r, Action a) :
		_receiver(r), _action(a)
	virtual void Execute() { 
		(_receiver->*Action)(); 
	}
};
*/

class MoveCommand : public Command {
protected:
	CElement* element_;
	D2D1_POINT_2F delta_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	MoveCommand(CElement* element, const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) 
		: element_(element)
		, delta_(delta)
		, qtRoot_(qtRoot)
	{
	}
	virtual void Execute();
	virtual void UnExecute();
};

class BlockMoveCommand : public MoveCommand {
protected:
	const bool was_modified_;
	std::vector<CLine*> lines_;
public:
	BlockMoveCommand(CBlock* block, 
		const D2D1_POINT_2F& delta, 
		CElement::MyQuadrantNode* qtRoot) 
		: MoveCommand(block, delta, qtRoot) 
		, was_modified_(block->top->is_modified()) 
	{
		lines_ = std::move(block->GetLines());
		block->top->set_modified();
		for (auto& s : block->top->slots) {
			s->set_modified();
		}
	}
	virtual void Execute();
	virtual void UnExecute();
};

class InsertLineCommand : public Command {
	CLine* line_;
	CElement::MyQuadrantNode* qtRoot_;
	CBlockComposition* blocks_;
public:
	InsertLineCommand(CLine* line, CBlockComposition* pBlocks, CElement::MyQuadrantNode* qtRoot) :
		line_(line), blocks_(pBlocks), qtRoot_(qtRoot) { }
	virtual ~InsertLineCommand();
	virtual void Execute();
	virtual void UnExecute();
};

class UpdateLinesCommand : public Command {
	std::vector<CLine*> lines_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	UpdateLinesCommand(std::vector<CLine*>& lines, CElement::MyQuadrantNode* qtRoot) :
		lines_(lines), qtRoot_(qtRoot) {
	}
	virtual void Execute();
	virtual void UnExecute();
};

class InsertBlockCommand : public Command {
	npsys::fbd_block_n block_;
	CBlockComposition* editor_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	InsertBlockCommand(
		npsys::fbd_block_n&& block, 
		CBlockComposition* editor, 
		CElement::MyQuadrantNode* qtRoot);
	virtual void Execute();
	virtual void UnExecute();
};

class PasteClipboardCommand : public Command {
	size_t inserted_count_;
	std::unique_ptr<CBlockCompositionWrapper> source_;
	CBlockComposition* dest_;
	CElement::MyQuadrantNode* qtRoot_;
	CBlocksInspector blocks_;
public:
	PasteClipboardCommand(
		std::unique_ptr<CBlockCompositionWrapper> source, 
		CBlockComposition* dest, 
		CElement::MyQuadrantNode* qtRoot);
	virtual ~PasteClipboardCommand();
	virtual void Execute();
	virtual void UnExecute();
};


class DeleteBlockCommand : public Command {
	npsys::fbd_block_n block_;
	CBlockComposition* editor_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	DeleteBlockCommand(
		npsys::fbd_block_n&& block, 
		CBlockComposition* editor, 
		CElement::MyQuadrantNode* qtRoot);
	virtual void Execute();
	virtual void UnExecute();
	virtual void ExecutePermanent();
};

class DeleteLineCommand : public Command {
	CLine* _pLine;
	CBlockComposition* editor_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	DeleteLineCommand(CLine* pLine, CBlockComposition* pBlocks, CElement::MyQuadrantNode* qtRoot);
	virtual ~DeleteLineCommand();
	virtual void Execute();
	virtual void UnExecute();
};



template<class T>
class GroupCommand : public Command
{
public:
	virtual ~GroupCommand() = default;
	void AddCommand(T* command, bool execute) {
		commands_.emplace_back(command);
		if (execute) command->Execute();
	}
	virtual void Execute() {
		for (auto &c : commands_) {
			if (!c->IsExecuted()) c->Execute();
		}
		executed_ = true;
	}
	virtual void UnExecute() {
		for (auto it = commands_.rbegin(); it != commands_.rend(); it++) (*it)->UnExecute();
		executed_ = false;
	}
	virtual void ExecutePermanent() {
		for (auto &c : commands_) c->ExecutePermanent();
	}
private:
	std::vector<std::unique_ptr<T>> commands_;
};

template<class T>
class GroupCommand2 : public GroupCommand<T>
{
	typedef GroupCommand<T> base;
public:
	GroupCommand2() : command_(nullptr) { }
	virtual ~GroupCommand2() {
		delete command_;
	}
	virtual void Execute() {
		base::Execute();
		command_->Execute();
	}
	virtual void UnExecute() {
		base::UnExecute();
		command_->UnExecute();
	}
	virtual void ExecutePermanent() {
		command_->ExecutePermanent();
		base::ExecutePermanent();
	}
	void SetFinishCommand(Command* command) {
		command_ = command;
	}
protected:
	Command* command_;
};

/*
class ChangeSlotTypeCommand : public Command
{
public:
	ChangeSlotTypeCommand(
		CSlot* pSlot,
		CParameter* pParam,
		PARAMETER_TYPE type,
		CSlotType* poldSt,
		CSlotType* pnewldSt,
		CBlockComposition* pBlocks);

	virtual void Execute();
	virtual void UnExecute();
	virtual void Delete();
private:
	bool executed_;
	CSlot* _pSlot;
	CParameter* _pParam;
	PARAMETER_TYPE _type;
	PARAMETER_TYPE _old_type;
	CSlotType *_pnewSt, *_poldSt;
	CBlockComposition* blocks_;
};
*/