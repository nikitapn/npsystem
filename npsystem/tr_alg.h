#pragma once

#include "tr_item.h"
#include <npsys/header.h>
#include "algext.h"
#include "tr_block.h"

class CTreeAlgorithm : public LazyItemListContainer<
	CTreeAlgorithm,
	std::vector, 
	CTreeBlock, 
	DatabaseElement<npsys::algorithm_n, npsys::algorithm_l>,
	Manual>
{
public:
	CTreeAlgorithm(npsys::algorithm_n& n, npsys::algorithm_l& l);
	virtual void ConstructMenu(CMenu* menu) noexcept;
	virtual void HandleRequest(REQUEST* req) noexcept;
	virtual INT_PTR ShowProperties();
	virtual bool ChangeName(const std::string& name);
	CItemView* CreateView(CMyTabView& tabview);
	int Move(CTreeItemAbstract* item, int action);
	virtual bool IsRemovable();
protected:
	virtual void LoadChildsImpl() noexcept;
	virtual void PostDelete(CContainer* parent) noexcept;
};