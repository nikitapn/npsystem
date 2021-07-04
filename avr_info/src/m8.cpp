// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>

namespace avrinfo::m8 {

static const RegInfo registers[]
{
	{ "PORTB",	0x38 },
	{ "DDRB",		0x37 },
	{ "PINB",		0x36 },
	{ "PORTC",	0x35 },
	{ "DDRC",		0x34 },
	{ "PINC",		0x33 },
	{ "PORTD",	0x32 },
	{ "DDRD",		0x31 },
	{ "PIND",		0x30 },
	{ "",				0xff }
};

static const char ports[] = { 'B', 'C', 'D', 0 };

static const ControllerPinsConfig pins[]
{
	{ 'B', 0, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 1, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 2, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 3, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 4, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 5, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },

	{ 'C', 0, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'C', 1, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'C', 2, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'C', 3, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },

	{ 'D', 6, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'D', 7, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },

	{ 0, 0, 0 }
};

PeripheralInfo pinfo{ registers, ports, pins };

} // namespace avrinfo::m8