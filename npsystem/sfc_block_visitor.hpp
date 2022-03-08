// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "visitorbase.h"
#include "line_connectable.hpp"

class CSFCFindConnectable
	: public CBlockVisitor
	, public COneTypeContainerSmallVector32<CLineConnectableT<CSFCLine>*> 
{
public:
	ACCEPT_DECL(CSFCBlockBegin)
	ACCEPT_DECL(CSFCBlockTerm)
	ACCEPT_DECL(CSFCBlockStep)
	ACCEPT_DECL(CSFCBlockTransition)
};

class CSFCFindSlots
	: public CBlockVisitor
	, public COneTypeContainerSmallVector32<CSFCSlot*> 
{
public:
	ACCEPT_DECL(CSFCInputSlot)
	ACCEPT_DECL(CSFCOutputSlot)
};

class CSFCFindGeometries
	: public CBlockVisitor
	, public COneTypeContainerUnorderedSet<IRecreatable*> 
{
	void Accept(CSFCSlot* slot);
public:
	ACCEPT_DECL(CSFCInputSlot)
	ACCEPT_DECL(CSFCOutputSlot)
};


class CSFCBlocksInspector 
	: public CBlockVisitor
	, public COneTypeContainerSmallVector32<CSFCBlock*> 
{
public:
	#define AAF(x) ACCEPT_DECL(x)
		NPSYSTEM_BLOCKS_SFC()
	#undef AAF
};

/*
class CSFCFindSelectedBlocks
	: public CBlockVisitor
	, public COneTypeContainerSmallVector32<CSFCBlock*> 
{
public:
	#define AAF(x) ACCEPT_DECL(x)
		NPSYSTEM_BLOCKS_SFC()
	#undef AAF
};

class CSFCFindSelectedPlusLines 
	: public CSFCFindSelectedBlocks 
{
public:
	std::vector<CSFCLine*> lines;
	
	ACCEPT_DECL(CSFCLine);
};
*/