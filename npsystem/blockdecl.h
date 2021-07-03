#pragma once

class CElement;
class CBlock;
class CConfigurableBlock;
class CLogicElement;
class CBlockComposition;
class CBlockCompositionWrapper;
// slots
class CSlot;
class CInputSlot;
class COutputSlot;
class CSlotGroup;
// line
class CLine;
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

#define ALPHA_BLOCKS() \
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
	

#define AAF(x) class x;
ALPHA_BLOCKS()
#undef AAF