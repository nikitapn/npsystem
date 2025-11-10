// Copyright (c) 2021-2025, Nikita Pennie <nikitapnn1@gmail.com>
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "environment.h"
#include "protocol_base.h"
#include "protocol_task.h"
#include "protocol_read.h"
#include "protocol_write.h"
#include "protocol_avr5.h"

extern std::unique_ptr<protocol::protocol_service> proto;
