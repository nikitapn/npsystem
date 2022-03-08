// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "element.h"

class CBlockComposition;

class Command_DeleteElementFromGroupSimple 
	: public Command {
	CElement* element_;
	CGroup* group_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	virtual void Execute();
	virtual void UnExecute();
	virtual void ExecutePermanent();

	Command_DeleteElementFromGroupSimple(CElement* element, CGroup* group, CElement::MyQuadrantNode* qtRoot);
};

class Command_InsertElementToGroupSimple 
	: public Command_DeleteElementFromGroupSimple {
public:
	virtual void Execute() { Command_DeleteElementFromGroupSimple::UnExecute(); }
	virtual void UnExecute() { Command_DeleteElementFromGroupSimple::Execute(); }
	virtual void ExecutePermanent() {}

	Command_InsertElementToGroupSimple(CElement* element, CGroup* group, CElement::MyQuadrantNode* qtRoot)
		: Command_DeleteElementFromGroupSimple(element, group, qtRoot) {}
};

class Command_DeleteElement 
	: public Command {
	CElement* element_;
	CBlockComposition* blocks_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	virtual void Execute();
	virtual void UnExecute();
	virtual void ExecutePermanent();

	Command_DeleteElement(CElement* element, CBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot);
};

class Command_InsertElement 
	: public Command_DeleteElement {
public:
	virtual void Execute() { Command_DeleteElement::UnExecute(); }
	virtual void UnExecute() { Command_DeleteElement::Execute(); }
	virtual void ExecutePermanent() {}
	
	Command_InsertElement(CElement* element, CBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot)
		: Command_DeleteElement(element, blocks, qtRoot) {}
};


class Command_MoveCommand 
	: public Command {
protected:
	CElement* element_;
	D2D1_POINT_2F delta_;
	CElement::MyQuadrantNode* qtRoot_;
public:
	virtual void Execute();
	virtual void UnExecute();
	
	Command_MoveCommand(CElement* element, const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot) 
		: element_(element), delta_(delta), qtRoot_(qtRoot) {}
};

template<class T>
class GroupCommand : public Command {
	std::vector<std::unique_ptr<T>> commands_;
public:
	template<typename U, typename... Args>
	void CreateCommand(bool execute, Args&&... args) {
		commands_.push_back(std::make_unique<U>(std::forward<Args>(args)...));
		if (execute) commands_.back()->Execute();
	}

	void AddCommand(std::unique_ptr<T>&& command, bool execute) {
		commands_.push_back(std::move(command));
		if (execute) commands_.back()->Execute();
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
};

template<class T>
class GroupCommand2 
	: public GroupCommand<T> 
{
	std::unique_ptr<T> last_command_;
public:
	virtual void Execute() {
		GroupCommand<T>::Execute();
		last_command_->Execute();
	}

	virtual void UnExecute() {
		GroupCommand<T>::UnExecute();
		last_command_->UnExecute();
	}

	virtual void ExecutePermanent() {
		last_command_->ExecutePermanent();
		GroupCommand<T>::ExecutePermanent();
	}

	GroupCommand2(std::unique_ptr<T>&& command) 
		: last_command_{std::move(command)} { }
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