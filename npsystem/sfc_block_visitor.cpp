#include "stdafx.h"
#include "sfc_block_visitor.hpp"
#include "sfc_block.hpp"

// CSFCFindConnectable

FINDSPECIFICTYPE_IMPL(CSFCFindConnectable, CSFCBlockBegin)
FINDSPECIFICTYPE_IMPL(CSFCFindConnectable, CSFCBlockTerm)
FINDSPECIFICTYPE_IMPL(CSFCFindConnectable, CSFCBlockStep)
FINDSPECIFICTYPE_IMPL(CSFCFindConnectable, CSFCBlockTransition)

// CSFCBlocksInspector

FINDSPECIFICTYPE_IMPL(CSFCBlocksInspector, CSFCBlockBegin)
FINDSPECIFICTYPE_IMPL(CSFCBlocksInspector, CSFCBlockTerm)
FINDSPECIFICTYPE_IMPL(CSFCBlocksInspector, CSFCBlockStep)
FINDSPECIFICTYPE_IMPL(CSFCBlocksInspector, CSFCBlockTransition)

// CSFCFindSlots

FINDSPECIFICTYPE_IMPL(CSFCFindSlots, CSFCInputSlot)
FINDSPECIFICTYPE_IMPL(CSFCFindSlots, CSFCOutputSlot)

// CSFCFindGeometries
void CSFCFindGeometries::Accept(CSFCInputSlot* slot) { Accept(static_cast<CSFCSlot*>(slot)); }
void CSFCFindGeometries::Accept(CSFCOutputSlot* slot)  { Accept(static_cast<CSFCSlot*>(slot)); }
void CSFCFindGeometries::Accept(CSFCSlot* slot) { slot->CollectGeometries(elems_); }

