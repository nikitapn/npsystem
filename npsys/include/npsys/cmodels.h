#pragma once

namespace npsys::hardware {
	enum class Model : int {
		ATMEGA8,
		ATMEGA8_VIRTUAL,
		ATMEGA16,
		ATMEGA16_VIRTUAL,
		AVR_MODEL_LAST = ATMEGA16_VIRTUAL,

		UNKNOWN_MODEL
	};
}