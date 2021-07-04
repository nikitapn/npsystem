// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/other/online_value.h>

class CContainer;

class COnlineTreeItem {
public:
	typedef void(*print_fun_t)(std::string&, uint32_t);
	enum Type {
		UNDEFINED = 500,
		VAR,
		BOOL,
		FUNCTION,
		HEX_UI1 = VARENUM::VT_UI1,
		HEX_UI2 = VARENUM::VT_UI2,
		HEX_UI4 = VARENUM::VT_UI4,
	};
	
	COnlineTreeItem(
		std::string _name,
		HICON _icon, 
		CContainer* _contaner = nullptr)
		: name(_name)
		, icon(_icon)
		, dev_addr(0xFF)
		, addr(0)
		, size(0)
		, vt(UNDEFINED)
		, fn(nullptr)
	{
		container = _contaner;
		childs_loaded = container ? false : true;
	}

	COnlineTreeItem(
		std::string _name,
		HICON _icon,
		uint8_t _dev_addr,
		size_t _addr,
		size_t _size,
		Type _vt,
		print_fun_t _fn = nullptr,
		CContainer* _contaner = nullptr)
		: name(_name)
		, icon(_icon)
		, dev_addr(_dev_addr)
		, addr(static_cast<uint16_t>(_addr))
		, size(static_cast<uint8_t>(_size))
		, vt(_vt)
		, fn(_fn)
	{
		ASSERT(vt != VAR);
		ASSERT(vt != UNDEFINED);
		container = _contaner;
		childs_loaded = container ? false : true;
	}

	COnlineTreeItem(
		std::string _name,
		HICON _icon,
		npsys::variable& var,
		CContainer* _contaner = nullptr)
		: name(_name)
		, icon(_icon)
		, fn(nullptr)
	{
		container = _contaner;
		childs_loaded = container ? false : true;
		dev_addr = var.GetDevAddr();
		addr = var.GetAddr();
		size = var.GetSize() + var.IsQuality();
		vt = VAR;
		online.set_type(var.GetType());
	}


	HTREEITEM Insert(CTreeViewCtrl tree, HTREEITEM parent) {
		tvis.hParent = parent;
		tvis.item.mask = TVIF_PARAM;
		tvis.item.lParam = (LPARAM)this;
		htr = tree.InsertItem(&tvis);
		if (container) {
			tvis.hParent = htr;
			tvis.item.mask = TVIF_PARAM;
			tvis.item.lParam = (LPARAM)this;
			htrFake = tree.InsertItem(&tvis);
		}
		return htr;
	}

	const std::string name;
	const HICON icon;
	uint8_t dev_addr;
	uint16_t addr;
	uint8_t size;
	Type vt;
	print_fun_t fn;
	HTREEITEM htr = NULL;
	online_value online;
	std::string online_st;
	bool childs_loaded;
	CContainer* container;
	HTREEITEM htrFake;
private:
	inline static TVINSERTSTRUCT tvis = { 0 };
};