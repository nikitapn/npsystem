// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>

namespace avrinfo::m16 {

static const RegInfo registers[] = 
{
	{ "PORTA",	0x3b },
	{ "DDRA",		0x3a },
	{ "PINA",		0x39 },
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

static const char ports[] = { 'A', 'B', 'C', 'D', 0 };

static const ControllerPinsConfig pins[] =
{
	{ 'A', 0, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 1, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 2, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 3, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 4, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 5, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 6, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },
	{ 'A', 7, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN | PinPurpose::ANALOG_PIN },

	{ 'B', 0, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 1, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 2, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 3, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 4, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 5, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 6, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'B', 7, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },

	{ 'C', 2, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'C', 3, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'C', 4, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'C', 5, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'C', 6, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'C', 7, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },

	{ 'D', 6, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },
	{ 'D', 7, PinPurpose::INPUTPU_PIN | PinPurpose::INPUT_PIN | PinPurpose::OUTPUT_PIN },

	{ 0, 0, 0 }
};

PeripheralInfo pinfo{ registers, ports, pins };

} // namespace avrinfo::m16