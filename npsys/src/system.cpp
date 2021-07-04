// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <npsys/system.h>

namespace npsys {
CSystem::CSystem(std::string name) 
	: odb::systree_item<CSystem>(name) 
{
	npsys::controllers_l controllers;
	controllers.init();
	controllers.store();
}

} // namespace npsys