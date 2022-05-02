#include "ast.hpp"

#include <npsys/network.h>
#include <npsys/control_unit.h>
#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include <npsys/fbdblock.h>

#include <npsys/cpp/cpp_variable.h>
#include <npsys/cpp/cpp_slot.h>

namespace npcompiler::ast {
	AstNode* ParserContext::ext_ident_get(std::string_view str) {
	//	if (auto founded = symbols_global.find(str); founded != symbols_global.end()) {
	//		return founded->second;
	//	}
	//
	//	auto slot = npsys::CFBDSlot::get_by_path(str);
	//	if (!slot->var.fetch()) throw std::string(str) + " is not defined.";
	//
	//	//slot->var->GetClearType();
		return nullptr;
	}
} // namespace npcompiler::ast