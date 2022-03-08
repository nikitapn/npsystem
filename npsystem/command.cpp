// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "command_impl.h"
#include "blockcomposition.hpp"

// Command_MoveCommand

void Command_MoveCommand::Execute() {
	RemoveFromQuadrant(element_, qtRoot_);
	element_->MoveDxDy(delta_.x, delta_.y);
	PasteToQuadrant(element_, qtRoot_);
}

void Command_MoveCommand::UnExecute() {
	RemoveFromQuadrant(element_, qtRoot_);
	element_->MoveDxDy(-delta_.x, -delta_.y);
	PasteToQuadrant(element_, qtRoot_);
}

// Command_DeleteElementFromGroupSimple

Command_DeleteElementFromGroupSimple::Command_DeleteElementFromGroupSimple(CElement* element, CGroup* group, CElement::MyQuadrantNode* qtRoot) 
	: element_(element), group_(group), qtRoot_(qtRoot)
{
}

void Command_DeleteElementFromGroupSimple::Execute() {
	RemoveFromQuadrant(element_, qtRoot_);
	group_->RemoveElement(element_);
}

void Command_DeleteElementFromGroupSimple::UnExecute() {
	group_->AddGraphicElement(element_);
	PasteToQuadrant(element_, qtRoot_);
}

void Command_DeleteElementFromGroupSimple::ExecutePermanent() {
	delete element_;
}

// Command_DeleteElement

Command_DeleteElement::Command_DeleteElement(CElement* element, CBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot) 
	: element_(element), blocks_(blocks), qtRoot_(qtRoot)
{
}

void Command_DeleteElement::Execute() {
	RemoveFromQuadrant(element_, qtRoot_);
	blocks_->RemoveElement(element_);
}

void Command_DeleteElement::UnExecute() {
	blocks_->Push(element_);
	PasteToQuadrant(element_, qtRoot_);
}

void Command_DeleteElement::ExecutePermanent() {
	delete element_;
}