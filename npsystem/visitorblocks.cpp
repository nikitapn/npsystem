#include "stdafx.h"
#include "visitorblocks.h"
#include "Block.h"
#include "Line.h"
#include "algext.h"

// CBlockVisitor
void CBlockVisitor::VisitSlotType(CSlot* slot) noexcept {
	current_slot_ = slot;
	auto slot_type = slot->GetSlotType();
	ASSERT(slot_type);
	slot_type->Visit(*this);
}

#define ACCEPT_APPLY_ACTION(Visitor, BlockClassName) \
	void Visitor ## ::Accept(BlockClassName* block) { \
		ApplyAction(static_cast<CVariable*>(block)); \
	}
//
FINDSPECIFICTYPE_IMPL(CLinesInspector, CLine)
//
#define AAF(x) FINDSPECIFICTYPE_IMPL(CBlocksInspector, x)
	ALPHA_BLOCKS()
#undef AAF
//

#define FINDSELECTED_IMPL(Visitor, BlockClassName) \
	void Visitor ## ::Accept(BlockClassName* pBlock) { \
		if ( pBlock->IsSelected() ) \
			elems_.push_back(pBlock); \
	}

#define AAF(x) FINDSELECTED_IMPL(CFindSelectedBlocks, x)
	ALPHA_BLOCKS()
#undef AAF
//
void CFindSelectedPlusLines::Accept(CLine* pLine) {
	if (pLine->IsSelected())
		m_lines.push_back(pLine);
}

// CFindElementsThatContainsVariables

#define FINDVARIABLES_IMPL(Visitor, BlockClassName) \
	void Visitor ## ::Accept(BlockClassName* pBlock) { \
		elems_.push_back(pBlock); \
		static_cast<CVariable*>(pBlock)->FillArray(m_all_variables); \
		m_vars.push_back(static_cast<CVariable*>(pBlock)); \
	}

// CFindVariableVisitorAbstract
void CFindVariableVisitorAbstract::Accept(COutputSlot* out) {
	VisitSlotType(out);
}
void CFindVariableVisitorAbstract::Accept(CInputSlot* in) {
	VisitSlotType(in);
}

ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CPositiveEdge)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CNegativeEdge)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CAnyEdge)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CDelay)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CPID)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CValue)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CExternalReference)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CBlockSchedule)
ACCEPT_APPLY_ACTION(CFindVariableVisitorAbstract, CCounter)

// CFindElementsThatContainsVariablesExludeExternalReferences
void CFindElementsThatContainsVariablesExcludeReferences::ApplyAction(CVariable* var) {
	elems_.push_back(var);
	var->FillArray(m_all_variables);
}

void CFindElementsThatContainsVariablesExcludeReferences::Accept(CAvrInternalPin*) {}
void CFindElementsThatContainsVariablesExcludeReferences::Accept(CModuleValue*) {}
void CFindElementsThatContainsVariablesExcludeReferences::Accept(CExternalReference*) {}

// CFindVariableNodes
void CFindVariableNodes::ApplyAction(CVariable* var) {
	var->FillArray(elems_);
}

// CResetVariables
void CResetVariables::ApplyAction(CVariable* var) {
	var->Clone(store_);
}

//void CResetVariables::Accept(COutputSlot* out) {
//	out->top->var = npsys::variable_n();
//	CHECK_SLOT_TYPE(out); 
//}
//void CResetVariables::Accept(CInputSlot* in) {
//	in->top->var = npsys::variable_n();
//	CHECK_SLOT_TYPE(in); 
//}

//  CReleaseVariables
void CReleaseVariables::ApplyAction(CVariable* var) {
	var->ReleaseMemory();
}

void CReleaseVariables::Accept(CExternalReference* ext) {
	ext->ReleaseMemory();
}

void CReleaseVariables::Accept(CAvrInternalPin* pin) {
	pin->ReleaseMemory();
}

void CReleaseVariables::Accept(CModuleValue* mod) {
	mod->ReleaseMemory();
}

// CFindInternalReferences
void CFindInternalReferences::Accept(COutputSlot* out) {
	auto size = elems_.size();
	VisitSlotType(out);
	if (size == elems_.size()) other_slots.push_back(out);
}
void CFindInternalReferences::Accept(CInputSlot* in) {
	auto size = elems_.size();
	VisitSlotType(in);
	if (size == elems_.size()) other_slots.push_back(in);
}
void CFindInternalReferences::Accept(CInternalRef* ref) {
	elems_.emplace_back(ref, current_slot_);
}

// CFindOutsideReferences
void CFindOutsideReferences::Accept(COutputSlot* out) {
	VisitSlotType(out);
}

void CFindOutsideReferences::Accept(CInputSlot* in) {
	VisitSlotType(in);
}

void CFindOutsideReferences::Accept(CExternalReference* ext) {
	elems_.push_back(ext);
}

void CFindOutsideReferences::Accept(CAvrInternalPin* pin) {
	elems_.push_back(pin);
}

void CFindOutsideReferences::Accept(CModuleValue* mod) {
	elems_.push_back(mod);
}

// CFindExternalReferences
void CFindExternalReferences::Accept(COutputSlot* out) {
	VisitSlotType(out);
}
void CFindExternalReferences::Accept(CInputSlot* in) {
	VisitSlotType(in);
}

void CFindExternalReferences::Accept(CExternalReference* ext) {
	elems_.emplace_back(ext, current_slot_);
	m_externalRef.emplace_back(ext, current_slot_);
	if (!ext->IsLinkValid()) m_invalidRef.emplace_back(ext, current_slot_);
}

void CFindExternalReferences::Accept(CAvrInternalPin* pin) {
	elems_.emplace_back(pin, current_slot_);
	m_pinRef.emplace_back(pin, current_slot_);
	if (!pin->IsLinkValid()) m_invalidRef.emplace_back(pin, current_slot_);
}

void CFindExternalReferences::Accept(CModuleValue* mod) {
	elems_.emplace_back(mod, current_slot_);
	m_modValue.emplace_back(mod, current_slot_);
	if (!mod->IsLinkValid()) m_invalidRef.emplace_back(mod, current_slot_);
}

// CFindSlots

void CFindSlots::Accept(COutputSlot* pSlot) {
	m_outSlots.push_back(pSlot);
	elems_.push_back(pSlot);
}
void CFindSlots::Accept(CInputSlot* pSlot) {
	m_inSlots.push_back(pSlot);
	elems_.push_back(pSlot);
}

void CFindSlots::AcceptConfigurableBlock(CConfigurableBlock* block) {
	for (auto& slot : block->disabled_slots_) {
		disabled_slots.push_back(slot);
	}
}

#define FIND_DISABLE_SLOT_IMPL(class_name) \
void CFindSlots::Accept(class_name* block) {\
	AcceptConfigurableBlock(block); }

FIND_DISABLE_SLOT_IMPL(CComparator);

// CFindLoadedSlots

template<class T>
inline void CFindLoadedSlots::AcceptSlot(T* slot) {
	auto var = slot->GetSlotAssociateVariable();
	if (!var) return;
	if (var->IsGood()) elems_.push_back(slot);
}

void CFindLoadedSlots::Accept(COutputSlot* slot) {
	AcceptSlot(slot);
}
void CFindLoadedSlots::Accept(CInputSlot* slot) {
	AcceptSlot(slot);
}

void CFindLoadedSlots::BeforeOnline() noexcept {
	for (auto slot : elems_) {
		slot->BeforeStartOnline();
	}
}

void CFindLoadedSlots::AfterOnline() noexcept {
	for (auto slot : elems_) {
		slot->AfterStopOnline();
	}
}

// CFindInternalBlockRef
FINDSPECIFICTYPE_IMPL(CFindInternalBlockRef, CTime)
FINDSPECIFICTYPE_IMPL(CFindInternalBlockRef, CBlockSchedule)

void CFindInternalBlockRef::InitInternalReferences(npsys::CNetworkDevice* device) {
	for (auto block : elems_) block->HardwareSpecific_Init(device);
}
void CFindInternalBlockRef::Clear() {
	for (auto block : elems_) block->HardwareSpecific_Clear();
}