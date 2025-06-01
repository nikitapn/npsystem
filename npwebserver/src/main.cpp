// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <cassert>

#include <nprpc/nprpc.hpp>
#include <nprpc_stub/nprpc_nameserver.hpp>

#include <nplib/utils/thread_pool.hpp>

#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>

#include <npsys/variable.h>
#include <npsys/web.h>
#include <npsys/variable.h>
#include "config.h"
#include <npc/npwebserver.hpp>

npwebserver::Config g_cfg;

using thread_pool = nplib::thread_pool_2;
using namespace std::chrono_literals;


class IWebServerImpl
	: public npwebserver::IWebServer_Servant {
public:
	virtual void get_web_categories(
		/*out*/nprpc::flat::Vector_Direct2<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct> catso) {
		npsys::web_l cats;
		cats.fetch_all_nodes();
		catso.length(cats.size());

		auto it = catso().begin();
		for (auto& cat : cats) {
			cat->items.fetch_all_nodes();
			(*it).name(cat->get_name());
			(*it).items(cat->items.size());
			auto item_it = (*it).items().begin();
			for (auto& webitem : cat->items) {
				(*item_it).name(webitem->get_name());
				(*item_it).description(webitem->description);
				if (webitem->slot.fetch()) {
					(*item_it).path(webitem->slot->path());
				} else {
					(*item_it).path("");
				}
				if (webitem->slot.fetch() && webitem->slot->var.fetch()) {
					(*item_it).dev_addr() = webitem->slot->var->GetDevAddr();
					(*item_it).mem_addr() = webitem->slot->var->GetAddr();
					(*item_it).type() = webitem->slot->var->GetType();
				} else {
					(*item_it).dev_addr() = 0xFF;
				}
				++item_it;
			}
			++it;
		}
	}
};


int main() {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif
	g_cfg.load("npwebserver");
	std::cout << g_cfg << std::endl;

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(thread_pool::get_instance().ctx(), SIGINT, SIGTERM);
	signals.async_wait(
		[&](boost::beast::error_code const&, int) {
			thread_pool::get_instance().stop();
		});

	IWebServerImpl web_server;

	try {
		auto rpc = nprpc::RpcBuilder()
			.set_debug_level(nprpc::DebugLevel::DebugLevel_Critical)
			.set_listen_tcp_port(g_cfg.socket_port)
			.set_listen_http_port(g_cfg.websocket_port)
			.set_http_root_dir(g_cfg.doc_root)
			.set_hostname("server.lan")
			.build(thread_pool::get_instance().ctx());

		auto poa = nprpc::PoaBuilder(rpc)
			.with_max_objects(2)
			.with_lifespan(nprpc::PoaPolicy::Lifespan::Persistent)
			.build();

		auto oid = poa->activate_object(&web_server, 
			nprpc::ObjectActivationFlags::ALLOW_TCP | nprpc::ObjectActivationFlags::ALLOW_WEBSOCKET);
		
		auto nameserver = rpc->get_nameserver(g_cfg.nameserver_ip);
		odb::Database::init(nameserver.get(), poa, g_cfg.data_dir() / "keys", "npwebserver");

		nameserver->Bind(oid, "npsystem_webserver");

		thread_pool::get_instance().ctx().run();
	} catch (std::exception& ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}

	std::cout << "server is shutting down..." << std::endl;

	return EXIT_SUCCESS;
}
