#pragma once

#include "tr_item.h"
#include <npsys/fbdblock.h>

class CTreeBlockValue : public CTemplateItem<
	CTreeBlockValue,
	CTreeItemAbstract,
	DatabaseElement<npsys::fbd_slot_n>> {
public:
	CTreeBlockValue(npsys::fbd_slot_n& n) 
	: CTemplateItem(n) {
		SetIcon(AlphaIcon::Software);
	}

	virtual void ConstructMenu(CMenu* menu) noexcept;
	virtual void HandleRequest(REQUEST* req) noexcept;
	virtual INT_PTR ShowProperties();

protected:
	virtual void PostDelete(CContainer* parent) noexcept {}; // prevent deletion from tree control
};

class CTreeBlock : public LazyItemListContainer<
	CTreeBlock,
	std::vector, 
	CTreeBlockValue,
	DatabaseElement<npsys::fbd_block_n>, 
	TableChilds> {
public:
	CTreeBlock(npsys::fbd_block_n& n)
		: LazyItemListContainer(n) {
		SetIcon(AlphaIcon::Block);
		n->tree_block = this;
	}
protected:
	virtual void PostDelete(CContainer* parent) noexcept {}; // prevent deletion from tree control
	virtual void LoadChildsImpl() noexcept;
};