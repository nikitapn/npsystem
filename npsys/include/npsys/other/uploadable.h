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