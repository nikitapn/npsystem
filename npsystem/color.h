// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

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

