#pragma once

#include <functional>
#include <string_view>
#include <npsys/variable.h>

namespace npcompiler {

	using globals_resolver_cb_t = std::function<npsys::variable_n(std::string_view)>;

};