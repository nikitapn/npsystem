// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"

#include <npdb/db.h>
#include <nprpc/nprpc.hpp>
#include <nprpc/nprpc_nameserver.hpp>

#include <nplib/utils/thread_pool.hpp>

#include <npcompiler/npcompiler.hpp>

#include <npsys/network.h>
#include <npsys/control_unit.h>
#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include <npsys/fbdblock.h>

#include <npsys/cpp/cpp_variable.h>
#include <npsys/cpp/cpp_slot.h>

using thread_pool = nplib::thread_pool_2;

int test_compiler() {
	nprpc::Rpc* rpc = nullptr;
	
	auto destroy = [&] {
		thread_pool::get_instance().stop();
		if (rpc) rpc->destroy();
	};

	try {
		nprpc::Config rpc_cfg;
		rpc_cfg.debug_level = nprpc::DebugLevel::DebugLevel_Critical;
		rpc_cfg.port = 24685;
		rpc_cfg.websocket_port = 0;

		rpc = nprpc::init(thread_pool::get_instance().ctx(), std::move(rpc_cfg));

		auto nameserver = rpc->get_nameserver("192.168.1.2");
		auto p1 = std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent);
		auto poa = rpc->create_poa(2, { p1.get() });
		odb::Database::init(nameserver.get(), poa, "./", "nptestdb");

		npcompiler::Compilation compilation(std::filesystem::path("c:/projects/cpp/npsystem/npcompiler/test/example0.txt"));
		return !(compilation.compile(
			[](std::string_view path)->npsys::variable_n {
				auto slot = npsys::CFBDSlot::get_by_path(path); 
				slot->var.fetch();
				return slot->var;
			}
		) == true);
		
		destroy();
	} catch (std::exception& ex) {
		std::cerr << ex.what();
		destroy();
		return -1;
	}
}