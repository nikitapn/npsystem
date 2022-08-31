// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nplib/utils/config.h>

NPLIB_REGISTER_SECTION(NPRPC);

NPLIB_DEFINE_CONFIG_STRUCT(
	(npwebserver), Config,
	(std::string, ip, (General, "127.0.0.1"))
	(unsigned short, socket_port, (General, 21020))
	(unsigned short, websocket_port, (General, 21021))
	(std::string, doc_root, (General, "\\\\wsl$\\Debian\\home\\png\\projects\\npweb\\public"))
	(std::string, nameserver_ip, (NPRPC, "192.168.1.2"))

)

extern npwebserver::Config g_cfg;