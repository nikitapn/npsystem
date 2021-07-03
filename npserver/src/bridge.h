#pragma once

#include "frame.h"

class bridge {
public:
	virtual int send_recive(const frame &output, frame &recived, size_t expected_length) noexcept = 0;
	virtual ~bridge() = default;
};


