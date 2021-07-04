// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nplib/utils/config.h>

NPLIB_REGISTER_SECTION(NPRPC);

NPLIB_DEFINE_CONFIG_STRUCT(
(npdbserver), Config,
	(std::string, db_name, (General, "npsystem_db"))
	(std::string, nameserver_ip, (NPRPC, "127.0.0.1"))
)

extern npdbserver::Config g_cfg;
