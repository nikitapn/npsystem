#pragma once

#include <nplib/utils/config.h>

NPLIB_REGISTER_SECTION(NPRPC);
NPLIB_REGISTER_SECTION(Client);
NPLIB_REGISTER_SECTION(Virtual);

NPLIB_DEFINE_CONFIG_STRUCT(
(npserver), Config,
	(int, log_level, (General, 3))
	(int, cmd_n_try, (General, 3))
	(int, read_period_ms, (General, 100))
	(int, timeout, (General, 100))
	(int, time_sync_period, (General, 300))
	(std::string, nameserver_ip, (NPRPC, "127.0.0.1"))
	(int, client_keep_alive_time, (Client, 5))
	(int, client_keep_alive_max_na, (Client, 3))
	(int, virtual_environment, (Virtual, 1))
)

extern npserver::Config g_cfg;
