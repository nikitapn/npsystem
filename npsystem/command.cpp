#include "stdafx.h"
#include "command.h"
#include "block.h"
#include "line.h"
#include "blockcomposition.h"
#include "algext.h"
#include "tr_block.h"
#include "tr_alg.h"

void MoveCommand::Execute() {
	RemoveFromQuadrant(element_, qtRoot_);
	element_->MoveDxDy(delta_.x, delta_.y);
	PasteToQuadrant(element_, qtRoot_);
}

void MoveCommand::UnExecute() {
	RemoveFromQuadrant(element_, qtRoot_);
	element_->MoveDxDy(-delta_.x, -delta_.y);
	PasteToQuadrant(element_, qtRoot_);
}

DeleteElementFromGroupSimple::DeleteElementFromGroupSimple(CElement* element, CGroup* group, CElement::MyQuadrantNode* qtRoot) 
	: element_(element)
	, group_(group)
	, qtRoot_(qtRoot)
{
}

void DeleteElementFromGroupSimple::Execute() {
	RemoveFromQuadrant(element_, qtRoot_);
	group_->RemoveElement(element_);
}

void DeleteElementFromGroupSimple::UnExecute() {
	group_->AddGraphicElement(element_);
	PasteToQuadrant(element_, qtRoot_);
}

void DeleteElementFromGroupSimple::ExecutePermanent() {
	delete element_;
}

// BlockMoveCommand
void BlockMoveCommand::Execute() {
	auto block = static_cast<CBlock*>(element_);
	for (auto line : lines_) RemoveFromQuadrant(line, qtRoot_);
	MoveCommand::Execute();
	block->UpdateBezierLines();
	for (auto line : lines_) PasteToQuadrant(line, qtRoot_);
	block->top->set_modified();
	for (auto& s : block->top->slots) {
		s->set_modified();
	}
}

void BlockMoveCommand::UnExecute() {
	auto block = static_cast<CBlock*>(element_);
	for (auto line : lines_) RemoveFromQuadrant(line, qtRoot_);
	MoveCommand::UnExecute();
	block->UpdateBezierLines();
	for (auto line : lines_) PasteToQuadrant(line, qtRoot_);
//	block->top->set_modified(was_modified_);
//	for (auto& s : block->top->slots) {
//		s->set_modified(was_modified_);
//	}
}
//
InsertLineCommand::~InsertLineCommand() {
	if (!executed_) delete line_;
}
void InsertLineCommand::Execute() {
	line_->Connect();
	line_->GetOut()->CalcLinesOffset();
	blocks_->Push(line_);
	executed_ = true;
	PasteToQuadrant(line_, qtRoot_);
}
void InsertLineCommand::UnExecute() {
	line_->GetIn()->DisconnectLine(line_);
	line_->GetOut()->DisconnectLine(line_);
	line_->GetOut()->CalcLinesOffset();
	blocks_->RemoveElement(line_);
	executed_ = false;
	RemoveFromQuadrant(line_, qtRoot_);
}
//
void UpdateLinesCommand::Execute() {
	for (auto& l : lines_) {
		qtRoot_->DeleteElement(l);
		l->Create();
		qtRoot_->AddElement(l);
	}
}
void UpdateLinesCommand::UnExecute() {
	Execute();
}
//
InsertBlockCommand::InsertBlockCommand(
	npsys::fbd_block_n&& block, 
	CBlockComposition* editor, 
	CElement::MyQuadrantNode* qtRoot)
	: block_(block)
	, editor_(editor)
	, qtRoot_(qtRoot) 
{
}

void InsertBlockCommand::Execute() {
	PasteToQuadrant(block_->e_block.get(), qtRoot_);
	auto alg = editor_->GetAlgorithm();
	block_->tree_block = new CTreeBlock(block_);
	alg->tree_algorithm->AddItem(block_->tree_block);
	editor_->AddBlock(block_);
	executed_ = true;
}

void InsertBlockCommand::UnExecute() {
	RemoveFromQuadrant(block_->e_block.get(), qtRoot_);
	editor_->PopBlock();
	block_->tree_block->Delete();
	block_->tree_block = nullptr;
	executed_ = false;
}

//DeleteBlockCommand
DeleteBlockCommand::DeleteBlockCommand(
	npsys::fbd_block_n&& block,
	CBlockComposition* editor,
	CElement::MyQuadrantNode* qtRoot)
	: block_(block)
	, editor_(editor)
	, qtRoot_(qtRoot) 
{
	block->e_block->SetSelect(false);
}
void DeleteBlockCommand::Execute() {
	RemoveFromQuadrant(block_->e_block.get(), qtRoot_);
	editor_->RemoveBlock(block_);
	if (block_->tree_block) {
		block_->tree_block->Delete();
		block_->tree_block = nullptr;
	}
	executed_ = true;
}

void DeleteBlockCommand::UnExecute() {
	PasteToQuadrant(block_->e_block.get(), qtRoot_);
	auto alg = editor_->GetAlgorithm();
	block_->tree_block = new CTreeBlock(block_);
	alg->tree_algorithm->AddItem(block_->tree_block);
	editor_->AddBlock(block_);
	executed_ = false;
}

void DeleteBlockCommand::ExecutePermanent() {
	block_->e_block->DeletePermanent();
	ATLASSERT(block_.use_count() == 1
	|| block_.use_count() == 2); // by this time it should be removed from all lists, but if block was previously inserted with insert command use_count will be 2
	block_.remove();
}

//DeleteLineCommand
DeleteLineCommand::DeleteLineCommand(
	CLine* pLine, 
	CBlockComposition* editor, 
	CElement::MyQuadrantNode* qtRoot) 
	: _pLine(pLine)
	, editor_(editor)
	, qtRoot_(qtRoot) 
{
	pLine->SetSelect(false);
}
DeleteLineCommand::~DeleteLineCommand() {
	if (executed_) delete _pLine;
}
void DeleteLineCommand::Execute() {
	_pLine->GetIn()->DisconnectLine(_pLine);
	_pLine->GetOut()->DisconnectLine(_pLine);
	editor_->RemoveElement(_pLine);
	executed_ = true;
	qtRoot_->DeleteElement(_pLine);
}

void DeleteLineCommand::UnExecute() {
	_pLine->GetIn()->AddLine(_pLine);
	_pLine->GetOut()->AddLine(_pLine);
	editor_->Push(_pLine);
	executed_ = false;
	qtRoot_->AddElement(_pLine);
}

// PasteClipboardCommand

PasteClipboardCommand::PasteClipboardCommand(
	std::unique_ptr<CBlockCompositionWrapper> source, 
	CBlockComposition* dest, 
	CElement::MyQuadrantNode* qtRoot) 
	: source_(std::move(source))
	, dest_(dest)
	, qtRoot_(qtRoot) 
{
	Traversal<CBlockVisitor>(source_.get(), blocks_);
}

PasteClipboardCommand::~PasteClipboardCommand() {
	if (!executed_) source_->DeleteElements();
}

void PasteClipboardCommand::Execute() {
	PasteToQuadrant(source_.get(), qtRoot_);
	inserted_count_ = dest_->Push(source_.get());
	auto alg = dest_->GetAlgorithm();
	for (auto& block : source_->blocks_) {
		block->tree_block = new CTreeBlock(block);
		alg->tree_algorithm->AddItem(block->tree_block);
		dest_->blocks_.add_node(block);
	}
	executed_ = true;
}

void PasteClipboardCommand::UnExecute() {
	RemoveFromQuadrant(source_.get(), qtRoot_);
	dest_->Pop(inserted_count_);
	for (auto& block : source_->blocks_) {
		dest_->blocks_.pop_back();
		block->tree_block->Delete();
		block->tree_block = nullptr;
	}
	executed_ = false;
}

// ChangeSlotTypeCommand
/*
ChangeSlotTypeCommand::ChangeSlotTypeCommand(
	CSlot* pSlot,
	CParameter* pParam,
	PARAMETER_TYPE type,
	CSlotType* poldSt,
	CSlotType* pnewldSt,
	CBlockComposition* pBlocks) {
	_pSlot = pSlot;
	_pParam = pParam;
	_type = type;
	_poldSt = poldSt;
	_pnewSt = pnewldSt;
	blocks_ = pBlocks;
	Execute();
}

void ChangeSlotTypeCommand::Execute() {
//	_pSlot->slot_type_ = _pnewSt;
	_old_type = _pParam->SetParamType(_type);
	executed_ = true;
}

void ChangeSlotTypeCommand::UnExecute() {
	_pSlot->slot_type_ = _poldSt;
	_type = _pParam->SetParamType(_old_type);
	executed_ = false;
}

void ChangeSlotTypeCommand::Delete() {
	if (executed_) delete _poldSt;
	else delete _pnewSt;
}
*/