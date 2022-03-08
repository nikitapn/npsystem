#pragma once

#include <npsys/fbdblock.h>
#include "block_factory.hpp"


class CFBDBlockComposition;
class CSFCBlockComposition;
class CSlotType;
class CBlock;
class CSlot;
class CSliderTimeChart;
class CSliderThing;
class CScheduleSlider;
class CBlockSchedule;

class CFBDBlockFactory : public CBlockFactory {
	void CreateBlock(CBlock* block);
	CScheduleSlider* CreateScheduleSlider(CBlockSchedule* parent);
	CSliderTimeChart* CreateSliderTimeChart(CScheduleSlider* parent);
	npsys::fbd_slot_n CreateSlot(const std::string& name, CSlot* slot, npsys::fbd_block_n& block);
	
	CFBDBlockComposition* blocks_;
public:
	npsys::fbd_block_n CreateBlock(int id);
	int	SetNewId(class CSlot*);
	void CreateInput(npsys::fbd_block_n&);
	void CreateOutput(npsys::fbd_block_n&);
	void CreateOr(npsys::fbd_block_n&);
	void CreateAnd(npsys::fbd_block_n&);
	void CreateRsTrigger(npsys::fbd_block_n&);
	void CreateNot(npsys::fbd_block_n&);
	void CreatePositiveEdge(npsys::fbd_block_n&);
	void CreateNegativeEdge(npsys::fbd_block_n&);
	void CreateAnyEdge(npsys::fbd_block_n&);
	void CreateDelay(npsys::fbd_block_n&);
	void CreateBinaryEncoder(npsys::fbd_block_n&);
	void CreateBinaryDecoder(npsys::fbd_block_n&);
	void CreateAdd(npsys::fbd_block_n&);
	void CreateSub(npsys::fbd_block_n&);
	void CreateMul(npsys::fbd_block_n&);
	void CreateDiv(npsys::fbd_block_n&);
	void CreateComparator(npsys::fbd_block_n&);
	void CreateFunction(npsys::fbd_block_n&);
	void CreatePID(npsys::fbd_block_n&);
	void CreateAlarmHigh(npsys::fbd_block_n&);
	void CreateAlarmLow(npsys::fbd_block_n&);
	void CreateTime(npsys::fbd_block_n&);
	void CreateSchedule(npsys::fbd_block_n&);
	void CreateCounter(npsys::fbd_block_n&);
	void CreatePulse(npsys::fbd_block_n&);
	npsys::fbd_slot_n CreateInputSlot(const std::string&, npsys::fbd_block_n&, CSlotType*);
	npsys::fbd_slot_n CreateOutputSlot(const std::string&, npsys::fbd_block_n&, CSlotType*);
	CSliderThing* CreateSliderThing(CScheduleSlider* parent);

	CFBDBlockFactory(CFBDBlockComposition* blocks) 
		: blocks_(blocks) 
	{
	}
};