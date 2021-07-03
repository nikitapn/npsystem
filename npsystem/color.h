#pragma once

enum class SolidColor {
	White,
	Black,
	Red,
	Green,
	Blue,
	Gray,
	LightGray,
	DarkGray,
	LineBadQuality,
	LineDeviceNotResponded,
	SlotValue,
	EditColor,
	SelColor,
	SliderThing,
	SliderThingHovered,
	SliderBackground,
	Test,
	NoColor
};
constexpr size_t SolidColorCount = 14;

enum class BlockColor {
	Standart,
	Parameter,
	ParameterQuality,
	Math,
	Compare,
	Control,
};
constexpr size_t BlockColorCount = 6;

