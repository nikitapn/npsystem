// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nplib/utils/config.h>

NPLIB_REGISTER_SECTION(NPRPC);

NPLIB_DEFINE_CONFIG_STRUCT(
	(npsystem), Config,
	(std::string, toolchain_dir, (General, "C:\\Program Files (x86)\\Atmel\\Studio\\7.0\\toolchain"))
	(std::string, avrdude_exe, (General, "D:\\programms\\avrdude\\avrdude.exe"))
	(std::string, avr_programmer, (General, "usbasp"))
	(int, server_timeout_sec, (General, 1))
	(std::string, nameserver_ip, (NPRPC, "127.0.0.1"))
)

extern npsystem::Config g_cfg;
