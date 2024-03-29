// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

class CControlWindow 
	: public CWindow {
public:
	CControlWindow(HWND hwnd = NULL) :
		CWindow(hwnd) {}

	virtual ~CControlWindow() = default;
};