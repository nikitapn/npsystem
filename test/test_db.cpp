// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"

#include <iostream>
#include <cassert>

#include <npsys/network.h>
#include <npsys/algsubcat.h>
#include <npsys/control_unit.h>

#include <nprpc/nprpc.hpp>
#include <nprpc/nprpc_nameserver.hpp>

#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>

#include <nplib/utils/thread_pool.hpp>

using thread_pool = nplib::thread_pool_2;
using namespace std::chrono_literals;

template<typename... Args>
void pr(Args&&... args) {
	//(std::invoke(printf, "%d\n", args)...);
	(printf("%d\n", args), ...);
}

int test_db() {
	pr(1, 2, 3, 4, 5);


	return 0;
	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(thread_pool::get_instance().ctx(), SIGINT, SIGTERM);
	signals.async_wait(
		[&](boost::beast::error_code const&, int) {
			thread_pool::get_instance().stop();
		});
	
	nprpc::Config rpc_cfg;
	rpc_cfg.debug_level = nprpc::DebugLevel::DebugLevel_Critical;
	rpc_cfg.port = 58463;
	rpc_cfg.websocket_port = 58464;

	auto rpc = nprpc::init(thread_pool::get_instance().ctx(), std::move(rpc_cfg));

	auto nameserver = rpc->get_nameserver("192.168.1.2");
	auto p1 = std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent);
	auto poa = rpc->create_poa(2, { p1.get() });
	odb::Database::init(nameserver.get(), poa, "./", "nptestdb");
	//rpc->start();

	npsys::algorithm_category_l algs;
	algs.fetch_all_nodes();

	for (auto& alg_cat : algs) {
		std::cerr << alg_cat->get_name() << '\n';
		alg_cat->units.fetch_all_nodes();
		for (auto& alg : alg_cat->units) {
			std::cerr << "  " << alg->get_name() << '\n';
		}
	}



	thread_pool::get_instance().stop();
	rpc->destroy();

	return 0;
}



/*
int main(int argc, char** argv) try {
	odb::Database::init(orb, root_poa, "key_file.key");

	
	std::thread corba_thread([&orb]() {
		orb->run();
	});

	int state = -1;
	int running = true;

	auto wait_for_enter = [] {
		std::cout << "press enter to continue\n";
		while (std::cin.get() != '\n');
	};

	auto get_node_id = [] {
		oid_t id;
		std::cout << "enter node id: ";
		std::cin >> id;
		return id;
	};
	
	while (running) {
		//std::system("cls");
		switch (state) {
		case 0:
			running = false;
			break;
		case 1: {
			test_node_n n;
			if (n.create()) {
				std::cout << "node created id: " << n.id() << std::endl;
			} else {
				std::cout << "failed to create node" << std::endl;
			}
			wait_for_enter();
			state = -1;
			break;
		}
		case 3: {
			test_node_n n(get_node_id());
			n.fetch();
			if (n.advise(node_callback)) {
				std::cout << "advised to node id: " << n.id() << std::endl;
			}
			wait_for_enter();
			state = -1;
			break;
		}
		case 4: {
			test_node_n n(get_node_id());
			n.fetch();
			if (n.remove()) {
				std::cout << "node has been deleted" << std::endl;
			}
			wait_for_enter();
			state = -1;
			break;
		};
		default:
			std::cout
				<< ">1. New node" << '\n'
				<< ">2. New node with name" << '\n'
				<< ">3. Advice" << '\n'
				<< ">4. Delete node" << '\n'
				<< ">0. Exit"
				<< std::endl;
			
			while (true) {
				int s = _getch() - '0';
				if (s >= 0 && s <= 4) {
					state = s;
					break;
				}
			}
		};
	}
	orb->destroy();
	corba_thread.join();
	return 0;
} catch (npd::DatabaseException& ex) {
	std::cerr << "CORBA::DatabaseException::code: " << ex.code << '\n';
	return -1;
} catch (CORBA::Exception& ex) {
	std::cerr << "CORBA::Exception:: " << ex._name() << '\n';
	return -1;
}
*/