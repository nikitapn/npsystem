#pragma once

#include "bridge.h"
#include <boost/noncopyable.hpp>

class bridge_avr_virtual : public bridge, public boost::noncopyable
{
public:
	virtual int send_recive(const frame &output, frame &recived, size_t expected_length) noexcept override;
};