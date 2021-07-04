// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npdb/memento.h>

class IUploadable {
//	friend class MementoManager;
//	template<class>
//	friend class NodeMementoManager;
public:
	std::unique_ptr<odb::IMemento> CreateMemento() noexcept { return CreateMemento_Uploadable(); }
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept = 0;
};