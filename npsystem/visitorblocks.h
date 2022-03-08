// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "visitorbase.h"
#include "line_connectable.hpp"
#include <npsys/variable.h>

class CFBDBlockVisitor : public CBlockVisitor {
protected:
	CSlot* current_slot_;
	void VisitSlotType(CSlot* slot) noexcept;
};


class CLinesInspector
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<CLine*> 
{
public:
	ACCEPT_DECL(CLine)
};

class CBlocksInspector 
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<CBlock*> 
{
public:
	#define AAF(x) ACCEPT_DECL(x)
		NPSYSTEM_BLOCKS()
	#undef AAF
};

class CFindSelectedBlocks
	: public CBlockVisitor
	, public COneTypeContainerSmallVector32<std::pair<CElement*, IConnectableToLine*>> 
{
public:
	// blocks
	#define AAF(x) ACCEPT_DECL(x)
		NPSYSTEM_BLOCKS()
		NPSYSTEM_BLOCKS_SFC()
	#undef AAF
};

class CFindSelectedPlusLines :
	public CFindSelectedBlocks 
{
public:
	std::vector<IRecreatable*> lines;
	
	ACCEPT_DECL(CLine);
	ACCEPT_DECL(CSFCLine);
};

class CVariable;

class CFindVariableVisitorAbstract
	: public CFBDBlockVisitor 
{
public:
	// Block
	ACCEPT_DECL(CPositiveEdge)
	ACCEPT_DECL(CNegativeEdge)
	ACCEPT_DECL(CAnyEdge)
	ACCEPT_DECL(CDelay)
	ACCEPT_DECL(CPID)
	ACCEPT_DECL(CBlockSchedule)
	ACCEPT_DECL(CCounter)
	ACCEPT_DECL(CPulse)
	// Slots
	ACCEPT_DECL(COutputSlot)
	ACCEPT_DECL(CInputSlot)
	// Slot types
	ACCEPT_DECL(CValue)
	ACCEPT_DECL(CExternalReference)

	virtual void ApplyAction(CVariable*) = 0;
};

class CFindVariableNodes
	: public CFindVariableVisitorAbstract
	, public COneTypeContainerVector<npsys::variable_n*> {
public:
	virtual void ApplyAction(CVariable*) override;
};

class CFindElementsThatContainsVariablesExcludeReferences
	: public CFindVariableVisitorAbstract
	, public COneTypeContainerSmallVector32<CVariable*> 
{
public:
	// simplified variable list
	var_node_ptr_list m_all_variables;

	virtual void ApplyAction(CVariable*) override;

	ACCEPT_DECL(CAvrInternalPin)
	ACCEPT_DECL(CModuleValue)
	ACCEPT_DECL(CExternalReference)
};

class CResetVariables
	: public CFindVariableNodes 
{
	bool store_;
public:
	explicit CResetVariables(bool store = false) 
		: store_(store) {}
	//	ACCEPT_DECL(COutputSlot);
	//	ACCEPT_DECL(CInputSlot);
	virtual void ApplyAction(CVariable*) override;
};

class CReleaseVariables
	: public CFindVariableNodes 
{
public:
	virtual void ApplyAction(CVariable*) override;

	ACCEPT_DECL(CAvrInternalPin)
	ACCEPT_DECL(CModuleValue)
	ACCEPT_DECL(CExternalReference)
};

class CFindInternalReferences
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<std::pair<CInternalRef*, CSlot*>> 
{
public:
	std::vector<CSlot*> other_slots;
	// Only param can contain this type of slot
	ACCEPT_DECL(COutputSlot)
	ACCEPT_DECL(CInputSlot)
	// Slot types
	ACCEPT_DECL(CInternalRef)
};

class CFindExternalReferences
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<std::pair<COutsideReference*, CSlot*>> 
{
public:
	std::vector<std::pair<COutsideReference*, CSlot*>> m_invalidRef;
	std::vector<std::pair<CExternalReference*, CSlot*>> m_externalRef;
	std::vector<std::pair<CAvrInternalPin*, CSlot*>> m_pinRef;
	std::vector<std::pair<CModuleValue*, CSlot*>> m_modValue;
	// Only param block can contain this type of slot
	ACCEPT_DECL(COutputSlot)
	ACCEPT_DECL(CInputSlot)
	// Slot types
	ACCEPT_DECL(CExternalReference)
	ACCEPT_DECL(CAvrInternalPin)
	ACCEPT_DECL(CModuleValue)
};

class CFindOutsideReferences
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<COutsideReference*> 
{
public:
	// Only param block can contain this type of slot
	ACCEPT_DECL(COutputSlot);
	ACCEPT_DECL(CInputSlot);
	// Slot types
	ACCEPT_DECL(CExternalReference)
	ACCEPT_DECL(CAvrInternalPin)
	ACCEPT_DECL(CModuleValue)
};

class CFindSlots 
	: public CFBDBlockVisitor
	, public COneTypeContainerVector<CSlot*> 
{
	void AcceptConfigurableBlock(CConfigurableBlock*);
public:
	std::vector<COutputSlot*> m_outSlots;
	std::vector<CInputSlot*> m_inSlots;
	std::vector<npsys::fbd_slot_n> disabled_slots;

	ACCEPT_DECL(COutputSlot)
	ACCEPT_DECL(CInputSlot)
	ACCEPT_DECL(CComparator)
};

class CFindLoadedSlots 
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<CSlot*> 
{
	template<class T>
	void AcceptSlot(T* slot);
public:
	void BeforeOnline() noexcept;
	void AfterOnline() noexcept;

	ACCEPT_DECL(COutputSlot)
	ACCEPT_DECL(CInputSlot)
};

class CInternalBlockRef;

class CFindInternalBlockRef
	: public CFBDBlockVisitor
	, public COneTypeContainerSmallVector32<CInternalBlockRef*> 
{
public:
	void InitInternalReferences(npsys::CNetworkDevice* device);
	void Clear();

	ACCEPT_DECL(CTime)
	ACCEPT_DECL(CBlockSchedule)
};


class CFBDFindConnectable
	: public CBlockVisitor
	, public COneTypeContainerSmallVector64<CLineConnectableT<CLine>*> 
{
public:
	ACCEPT_DECL(CInputSlot)
	ACCEPT_DECL(COutputSlot)
};