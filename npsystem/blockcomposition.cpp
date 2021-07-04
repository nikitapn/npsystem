// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "block.h"
#include "line.h"
#include "command.h"
#include "blockcomposition.h"
#include "algext.h"
#include <npsys/guid.h>

NP_BOOST_CLASS_EXPORT_GUID(CBlockComposition);
IMPLEMENT_VISITOR(CBlockComposition);

NP_BOOST_CLASS_EXPORT_GUID(CBlockCompositionWrapper);
IMPLEMENT_VISITOR(CBlockCompositionWrapper);


CElement::_Iterator* CBlockCompositionBase::CreateIterator() {
	m_iterator.SetRange(elements_.begin(), elements_.end());
	return static_cast<CElement::_Iterator*>(&m_iterator);
	//	return new Iter::StlContainerIterator<CElement*, std::vector>(_elements);
}
void CBlockCompositionBase::RestorePointers() {
	CFindSlots slots;
	CFindType<CLine, CBlockVisitor> lines;
	CFindInternalReferences refSlots;
	Traversal<CBlockVisitor>(this, { &slots, &lines, &refSlots });
	// Restore lines
	CSlot* founded[2];
	for (auto& l : lines) {
		int ix = 0;
		for (auto& j : slots) {
			int slotId = j->GetId();
			if (l->GetInID() == slotId ||
				l->GetOutID() == slotId) {
				founded[ix++] = j;
				if (ix == 2) break;
			}
		}
		if (ix < 2) {
			RemoveElement(l);
			delete l;
		} else {
			l->SetSlot(founded[0]);
			l->SetSlot(founded[1]);
			l->Connect();
		}
	}

	for (auto& osl : slots.m_outSlots)
		osl->CalcLinesOffset();

	// Restore Internal References pointers
	for (auto& rs : refSlots) {
		for (auto& s : slots) {
			if (rs.first->GetSlotID() == s->GetId()) {
				rs.first->SetInternalLink(s);
				break;
			}
		}
	}
}

void CBlockCompositionBase::UpdateAllBezierLines() {
	CFindType<CLine, CBlockVisitor> lines;
	Traversal<CBlockVisitor>(this, lines);
	for (auto& l : lines)
		l->Create();
}

void CBlockCompositionBase::ClearVariables() {
	CResetVariables cleaner;
	Traversal<CBlockVisitor>(this, cleaner);
}

void CBlockCompositionBase::Move(float x, float y) {
	D2D1_POINT_2F center = { 0 };
	for (auto& el : elements_) {
		center.x += el->GetPosition().x;
		center.y += el->GetPosition().y;
	}
	float n = static_cast<float>(elements_.size());
	MoveDxDy(x - center.x / n, y - center.y / n);
}

void CBlockCompositionBase::MoveDxDy(float dx, float dy) {
	for (auto& el : elements_) {
		el->MoveDxDy(dx, dy);
	}
}

// CBlockComposition

__declspec(noinline) void cpp_command_delete::operator()(Command* cmd) {
	delete cmd;
}

CBlockComposition::~CBlockComposition() {
	CFindSlots slots;
	Traversal<CBlockVisitor>(this, slots);
	for (auto slot : slots) slot->ClearReferences();
	for (auto i : elements_) {
		if (!i->IsNode()) delete i;
	}
#ifdef DEBUG
	// std::cout << "~CBlockComposition()" << std::endl;
#endif // DEBUG
}

CSlot* CBlockComposition::GetSlotById(int id) {
	CFindSlots slots;
	Traversal<CBlockVisitor>(this, slots);
	for (auto& s : slots) {
		if (s->GetId() == id) return s;
	}
	return nullptr;
}

std::string CBlockCompositionBase::CreateUniqueName(
	ConstElementIterator begin, 
	ConstElementIterator end, 
	const std::string& pattern) {
	std::string test = pattern + "_";
	std::string tmp;
	int i = 1;
	for (;;) {
		tmp = test + std::to_string(i++);
		if (std::find_if(begin, end, [&tmp](const CElement* e) {
			return e->GetName() == tmp; } 
		) == end) break;
	}
	return tmp;
}

std::string CBlockCompositionBase::CreateUniqueName(const std::string& pattern) const {
	return CBlockCompositionBase::CreateUniqueName(elements_.cbegin(), elements_.cend(), pattern);
}

bool CBlockComposition::CheckName(const std::string& name, CElement* pElem) {
	for (auto& el : *this) {
		if (el == pElem)
			continue;
		if (el->GetName() == name)
			return false;
	}
	return true;
}

void CBlockComposition::AddCommand(Command* cmd, bool bExecute) {
	if (cc_ != commands_.size() - 1) {
		auto it = commands_.begin();
		std::advance(it, cc_ + 1);
		commands_.erase(it, commands_.end());
	}
	commands_.push_back(std::unique_ptr<Command, cpp_command_delete>(cmd));
	if (bExecute) cmd->Execute();
	cc_ = (int)commands_.size() - 1;
}
int CBlockComposition::Undo() {
	if (IsUndoCommands()) {
		commands_[cc_--]->UnExecute();
	}
	return cc_;
}

void CBlockComposition::Redo() {
	int size = static_cast<int>(commands_.size());
	if ((cc_ < size - 1) && size) {
		commands_[++cc_]->Execute();
	}
}

bool CBlockComposition::IsRedoCommands() {
	int sz = static_cast<int>(commands_.size());
	return (cc_ < sz - 1) && sz;
}

bool CBlockComposition::IsUndoCommands() {
	return cc_ != -1;
}

void CBlockComposition::PermanentExecuteCommands() noexcept {
	if (IsUndoCommands()) {
		for (int ix = 0; ix < cc_ + 1; ix++) {
			commands_[ix]->ExecutePermanent();
		}
	}
	commands_.clear();
	cc_ = -1;
}

void CBlockComposition::DeleteSelectedElements(MyQuadrantNode* qtRoot) {
	CFindSelectedPlusLines sel;

	for (auto i : elements_) i->Visit(sel); // only top elements
	if (sel.empty() && sel.m_lines.empty()) return;

	auto group_cmd = new GroupCommand<Command>;
	
	// lines must be deleted first
	for (auto l : sel.m_lines) group_cmd->AddCommand(l->DeleteElement(this, qtRoot), true);
	for (auto i : sel) group_cmd->AddCommand(i->DeleteElement(this, qtRoot), false);

	AddCommand(group_cmd, true);
}

void CBlockComposition::SetSelect(bool select) noexcept {
	for (auto& el : elements_)
		el->SetSelect(select);
}
// CBlockCompositionWrapper
void CElement::InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept {
	static_cast<CBlockCompositionBase*>(wrapper)->Push(this);
}
void CBlock::InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept {
	wrapper->PushBlock(this->top);
}
void CBlockCompositionWrapper::Push(CElement* element) noexcept {
	element->InsertToWrapper(this);
}
void CBlockCompositionWrapper::PushBlock(npsys::CFBDBlock* block) {
	blocks_.push_back(block->self_node.fetch());
}
void CBlockCompositionWrapper::ReplaceElement(CElement* old_element, npsys::fbd_block_n& new_block) {
	int n = 0;
	for (auto& ptr : elements_) {
		if (ptr == old_element) {
			ptr = new_block->e_block.get();
			n += 1;
			break;
		}
	}

	for (auto& block : blocks_) {
		if (block->e_block.get() == old_element) {
			block = new_block;
			n += 1;
		}
	}

	if (n != 2) assert(false);
}
void CBlockCompositionWrapper::Adapt(CBlockComposition* dest) {
	auto dest_alg = dest->GetAlgorithm()->self_node.fetch();
	auto factory = dest->GetBlockFactory();

	CFindOutsideReferences outRef;
	CFindInternalReferences intRef;
	CFindSlots dest_slots;
	CBlocksInspector new_blocks;

	Traversal<CBlockVisitor>(this, { &intRef, &outRef, &new_blocks });
	Traversal<CBlockVisitor>(dest, dest_slots);

	for (auto& block : new_blocks) {
		auto n = block->top->self_node.fetch();
		for (auto& slot : block->top->slots) {
			slot->block_parent = n;
		}
	}

	for (auto& out : outRef) {
		out->alg_ = dest_alg;
	}

	using iterator_t = std::vector<CSlot*>::iterator;
	auto set_ref_slot = [](oid_t oldId, CSlot* slot, iterator_t begin, iterator_t end) -> bool {
		auto founded = std::find_if(begin, end, [&oldId](const CSlot* slot) {
			return slot->GetId() == oldId;
		});
		if (founded != end) {
			slot->SetSlotType(new CInternalRef(*founded));
			slot->CommitTypeChanges();
			return true;
		}
		return false;
	};

	bool source_alg_valid = alg_.fetch();

	
	auto convert_to_external_ref = [&](CSlot* slot, CInternalRef* ref) {
		std::optional<npsys::fbd_slot_n> ref_slot;
		if (source_alg_valid) ref_slot = alg_->FindSlot(ref->GetSlotID());
		if (!source_alg_valid || !ref_slot) {
			static_cast<CParameter*>(slot->GetParentBlock())->SetParamType(PARAMETER_TYPE::P_VALUE);
			slot->SetSlotType(new CValue(npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY));
			slot->CommitTypeChanges();
			return true;
		}
		auto ext_ref = new CExternalReference(ref_slot.value(), alg_, dest_alg);
		auto param = static_cast<CParameter*>(slot->GetParentBlock());
		if (param->GetDirection() == CParameter::PARAMETER_DIRECTION::OUTPUT_PARAMETER) {
			// always make external read links
			npsys::fbd_block_n binput;
			factory->CreateInput(binput);
			
			ReplaceElement(param, binput);
		
			param = static_cast<CInput*>(binput->e_block.get());
			slot = param->GetSlot();
		}
		param->SetParamType(PARAMETER_TYPE::P_EXTERNAL_REF);
		slot->SetSlotType(ext_ref);
		slot->CommitTypeChanges();
		return true;
	};

	bool same_alg = (dest_alg.id() == alg_.id());
	for (auto[ref, ref_slot] : intRef) {
		auto oldId = ref->GetSlotID();
		if (
			set_ref_slot(oldId, ref_slot, intRef.other_slots.begin(), intRef.other_slots.end()) ||
			(same_alg 
				? set_ref_slot(oldId, ref_slot, dest_slots.begin(), dest_slots.end())
				: convert_to_external_ref(ref_slot, ref))
			) {
			// founded
		} else {
			// invalid link was copied. not an error
			//ASSERT(FALSE);
		}
		ref_slot->ResetState();
		factory->SetNewId(ref_slot);
	}
	for (auto slot : intRef.other_slots) {
		slot->ResetState();
		factory->SetNewId(slot);
	}
}

void CBlockCompositionWrapper::DeleteElements() {
	for (auto& el : elements_) {
		if (!el->IsNode()) delete el;
	}
}