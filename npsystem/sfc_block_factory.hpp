#pragma once

#include "block_factory.hpp"

class CSFCBlockBegin;
class CSFCBlockTerm;
class CSFCBlockStep;
class CSFCBlockTransition;

class CSFCBlockFactory : public CBlockFactory {
	CSFCBlockComposition* blocks_;

	template<typename T>
	void CreateSlot(const D2D1::MyPoint2F& position_middle, CSFCBlock* block);
public:
	CSFCBlock* CreateBlock(int id);

	CSFCBlockBegin* CreateBlockBegin();
	CSFCBlockTerm* CreateBlockTerm();
	CSFCBlockStep* CreateBlockStep();
	CSFCBlockTransition* CreateBlockTransition();

	CSFCBlockFactory(CSFCBlockComposition* blocks) 
		: blocks_(blocks) {}
};