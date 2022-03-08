// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory#pragma once

#include <npsys/header.h>
#include "tr_item.h"

class CTreeNetwork : 
	public ItemListContainer<CTreeNetwork, std::vector, CTreeItemAbstract, FixedElement, Manual> {
	npsys::controllers_l controllers_;

	void AppendController(const std::string& type);
public:
	CTreeNetwork() {
		SetIcon(NPSystemIcon::Network);
		name_ = "NETWORK";
		LoadChilds();
	}
	virtual oid_t GetId() const noexcept final;
	virtual void HandleRequest(REQUEST* req) noexcept final;
	virtual void ConstructMenu(CMenu* menu) noexcept;
	virtual bool IsRemovable() { return false; }
protected:
	virtual void LoadChildsImpl() noexcept;
};