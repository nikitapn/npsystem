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