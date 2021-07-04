// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "frame.h"

class bridge {
public:
	virtual int send_recive(const frame &output, frame &recived, size_t expected_length) noexcept = 0;
	virtual ~bridge() = default;
};


