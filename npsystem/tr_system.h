// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "tr_item.h"
#include "npsys/system.h"

void NPSystemCompileLibraries();

class CTreeWebRoot;
class CTreeStringsCat;
class CTreeModules;
class CTreeAlgorithms;
class CTreeNetwork;

class CTreeSystem : public ItemListContainer<CTreeSystem, std::vector, CTreeItemAbstract, DatabaseElement<npsys::system_n>> {
	using items_t = std::tuple<CTreeWebRoot, CTreeStringsCat, CTreeModules, CTreeAlgorithms, CTreeNetwork>;
public:
	CTreeSystem(npsys::system_n& n);
	virtual ~CTreeSystem();

	template<typename T>
	T* Get() noexcept {
		if constexpr (mp11::mp_contains<items_t, T>::type::value) {
			return static_cast<T*>(m_items[mp11::mp_find<items_t, T>::value]);
		} else {
			static_assert(false, "The System doesn't have item of type T");
		}
	}

	void CalcStatusAll();
protected:
	virtual void LoadChildsImpl() noexcept;
	virtual void PostDelete(CContainer* parent) noexcept {}
	virtual bool IsRemovable() { return false; }
};
