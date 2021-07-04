// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <sim/import_export.h>
#include <memory>

namespace sim {
class VM {
public:
	SIM_IMPORT_EXPORT
	VM();
	SIM_IMPORT_EXPORT
	void run();
	SIM_IMPORT_EXPORT
	~VM();
private:
	class Impl;
	std::unique_ptr<Impl> impl_;
};

} // namespace sim