// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "tr_item.h"
#include "network_ext.h"

class CTreeHardware;
class CTreeAssignedAlgorithmCat;
class CTreeAssignedI2CModulesCat;

class CTreeController : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeController, std::vector, CTreeItemAbstract,
	DatabaseElement<npsys::controller_n, npsys::controllers_l>, Manual>,
	CTreeController> {
public:
	CTreeController(npsys::controller_n& n, npsys::controllers_l& l);
	virtual void ConstructMenu(CMenu* menu) noexcept;
	virtual CItemView* CreateView(CMyTabView& tabview) final;
	virtual void HandleRequest(REQUEST* req) noexcept;
	CTreeHardware* GetHardware() noexcept;
	CTreeAssignedAlgorithmCat* GetSoftware() noexcept;
	CTreeAssignedI2CModulesCat* GetI2CModules() noexcept;
	virtual int	Move(CTreeControlUnit* alg, int action);
	virtual int	Move(CTreeI2CModule* mod, int action);
	virtual INT_PTR ShowProperties();
	virtual COnlineTreeItem* CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent);
protected:
	virtual void LoadChildsImpl() noexcept;
	virtual void PostDelete(CContainer* parent) noexcept;
	virtual bool IsRemovable();
};