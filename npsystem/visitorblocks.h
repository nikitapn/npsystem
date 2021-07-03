#pragma once

#include "visitorbase.h"
#include "blockdecl.h"
#include <npsys/variable.h>

class CBlockVisitor {
protected:
	CSlot* current_slot_;

	void VisitSlotType(CSlot* slot) noexcept;
public:
	typedef CElement* InterfacePtr;

	virtual ~CBlockVisitor() = default;

	ACCEPT_NONE(CBlockComposition)
	ACCEPT_NONE(CBlockCompositionWrapper)
	// slots
	ACCEPT_NONE(CInputSlot)
	ACCEPT_NONE(COutputSlot)
	ACCEPT_NONE(CSlotGroup)
	// lines
	ACCEPT_NONE(CLine)
	// blocks
	#define AAF(x) ACCEPT_NONE(x)
		ALPHA_BLOCKS()
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
};

class CLinesInspector
	: public CBlockVisitor
	, public COneTypeContainer<CLine*> 
{
public:
	ACCEPT_DECL(CLine)
};

class CBlocksInspector 
	: public CBlockVisitor
	, public COneTypeContainer<CBlock*> 
{
public:
	#define AAF(x) ACCEPT_DECL(x)
		ALPHA_BLOCKS()
	#undef AAF
};

class CFindSelectedBlocks
	: public CBlockVisitor
	, public COneTypeContainer<CBlock*> 
{
public:
	// blocks
	#define AAF(x) ACCEPT_DECL(x)
		ALPHA_BLOCKS()
	#undef AAF
};

class CFindSelectedPlusLines :
	public CFindSelectedBlocks 
{
public:
	std::vector<CLine*> m_lines;
	
	ACCEPT_DECL(CLine);
};

class CVariable;

class CFindVariableVisitorAbstract
	: public CBlockVisitor 
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
	, public COneTypeContainer<npsys::variable_n*> {
public:
	virtual void ApplyAction(CVariable*) override;
};

class CFindElementsThatContainsVariablesExcludeReferences
	: public CFindVariableVisitorAbstract
	, public COneTypeContainer<CVariable*> 
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
	: public CBlockVisitor
	, public COneTypeContainer<std::pair<CInternalRef*, CSlot*>> 
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
	: public CBlockVisitor
	, public COneTypeContainer<std::pair<COutsideReference*, CSlot*>> 
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
	: public CBlockVisitor
	, public COneTypeContainer<COutsideReference*> 
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
	: public CBlockVisitor
	, public COneTypeContainer<CSlot*> 
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
	: public CBlockVisitor
	, public COneTypeContainer<CSlot*> 
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
	: public CBlockVisitor
	, public COneTypeContainer<CInternalBlockRef*> 
{
public:
	void InitInternalReferences(npsys::CNetworkDevice* device);
	void Clear();

	ACCEPT_DECL(CTime)
	ACCEPT_DECL(CBlockSchedule)
};