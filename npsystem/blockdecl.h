// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

class CElement;
class CBlock;
class CSFCBlock;
class CConfigurableBlock;
class CLogicElement;
class CFBDBlockComposition;
class CSFCBlockComposition;
class CBlockCompositionWrapper;
// slots
class CSlot;
class CInputSlot;
class COutputSlot;
class CSlotGroup;
// line
class CLineBase;
class CLine;
class CSFCLine;
class CSFCBus;
class CSFCBusIn;
class CSFCBusOut;
// Slot types
class CBlockInput;
class CValue;
class CValueRef;
class CBitRef;
class CInternalRef;
class COutsideReference;
class CExternalReference;
class CAvrInternalPin;
class CModuleValue;
class CFixedValue;
//
class CTextElement;
class CStaticTextElement;
class CDxEdit;
class CInplaceBlockProperty;
class CScheduleSlider;
class CSliderThing;
class CSliderTimeChart;
class CSizeElement;
class CSFCSlot;
class CSFCInputSlot;
class CSFCOutputSlot;

#define NPSYSTEM_BLOCKS() \
	AAF(CInput) \
	AAF(COutput) \
	/*logic*/ \
	AAF(COr) \
	AAF(CAnd) \
	AAF(CRsTrigger) \
	AAF(CNot) \
	AAF(CBinaryEncoder) \
	AAF(CBinaryDecoder) \
	AAF(CPositiveEdge) \
	AAF(CNegativeEdge) \
	AAF(CAnyEdge) \
	AAF(CDelay) \
	AAF(CTime) \
  AAF(CBlockSchedule) \
  AAF(CCounter) \
  AAF(CPulse) \
	/*Math*/ \
	AAF(CAdd) \
	AAF(CSub) \
	AAF(CMul) \
	AAF(CDiv) \
	AAF(CComparator) \
	AAF(CBlockFunction) \
	/*alarm*/ \
	AAF(CAlarmHigh) \
	AAF(CAlarmLow) \
	/*control*/ \
	AAF(CPID)

#define NPSYSTEM_BLOCKS_SFC() \
	AAF(CSFCBlockBegin) \
	AAF(CSFCBlockTerm) \
	AAF(CSFCBlockStep) \
	AAF(CSFCBlockTransition)


#define AAF(x) class x;
NPSYSTEM_BLOCKS()
NPSYSTEM_BLOCKS_SFC()
#undef AAF