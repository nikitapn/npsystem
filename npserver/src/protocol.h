#pragma once

#include "environment.h"
#include "protocol_base.h"
#include "protocol_task.h"
#include "protocol_read.h"
#include "protocol_write.h"
#include "protocol_avr5.h"

extern std::unique_ptr<protocol::protocol_service> proto;
