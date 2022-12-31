// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <cassert>
#include <Windows.h>
#include <fstream>
#include <filesystem>

#include <nplib/utils/thread_pool.hpp>
#include <nprpc/serialization/serialization.h>
#include <nprpc/serialization/oarchive.h>

#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>

#include "benchmark.hpp"

namespace nprpc::serialization {

template<class Archive>
void serialize(Archive& ar, benchmark::Data1& x) {
	ar& NVP2("f1", x.f1);
	ar& NVP2("f2", x.f2);
}

template<class Archive>
void serialize(Archive& ar, benchmark::Data& x) {
	ar& NVP2("f1", x.f1);
	ar& NVP2("f2", x.f2);
	ar& NVP2("f3", x.f3);
	ar& NVP2("f4", x.f4);
	ar& NVP2("f5", x.f5);
	ar& NVP2("f6", x.f6);
}

} // namespace nprpc::serialization


using thread_pool = nplib::thread_pool_2;
using namespace std::chrono_literals;

class Benchmark
	: public benchmark::IBenchmark_Servant 
{
	benchmark::Data raw_data;
	std::string json_data;
	std::stringstream ss;
public:
	virtual benchmark::Data rpc() { 
		return raw_data;
	}
	
	virtual std::string json() {
		ss.seekp(std::ios_base::beg);
		nprpc::serialization::json_oarchive oa(ss);
		oa << raw_data;
		return ss.str();
	}

	Benchmark() {
		srand(time(0));
		raw_data.f1 = "FIELD_F1";
		raw_data.f2 = "FIELD_F2";
		raw_data.f3 = 10;
		raw_data.f4 = 100;
		raw_data.f5 = 1000;
		raw_data.f6.resize(500);

		for (auto& x : raw_data.f6) {
			x.f1 = "F1_" + std::to_string(rand());
			x.f2 = (rand() % 500000) / 500000.0f;
		}

		std::stringstream ss;
		nprpc::serialization::json_oarchive oa(ss);
		oa << raw_data;

		json_data = ss.str();

		std::cerr << json_data;
	}
};

struct HostJson {
	bool secured;

	struct {
		nprpc::ObjectId benchmark;

		template<typename Archive>
		void serialize(Archive& ar) {
			ar& NVP(benchmark);
		}
	} objects;

	template<typename Archive>
	void serialize(Archive& ar) {
		ar& NVP(secured);
		ar& NVP(objects);
	}
};

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(thread_pool::get_instance().ctx(), SIGINT, SIGTERM);
	signals.async_wait(
		[&](boost::beast::error_code const&, int) {
			thread_pool::get_instance().stop();
		});

	HostJson host_json;
	Benchmark server;

	std::string http_root = "\\\\wsl$\\Arch\\home\\nikita\\projects\\nprpc_benchmark\\public";

	try {
		nprpc::Config rpc_cfg;
		rpc_cfg.debug_level = nprpc::DebugLevel::DebugLevel_Critical;
		rpc_cfg.websocket_port = 8080;
		rpc_cfg.http_root_dir = http_root;

		auto rpc = nprpc::init(thread_pool::get_instance().ctx(), std::move(rpc_cfg));
		auto p1 = std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent);
		auto poa = rpc->create_poa(2, { p1.get() });

		host_json.secured = false;
		host_json.objects.benchmark = poa->activate_object(&server);

		{
			std::ofstream os(std::filesystem::path(http_root) / "host.json");
			nprpc::serialization::json_oarchive oa(os);
			oa << host_json;
		}

		thread_pool::get_instance().ctx().run();
	} catch (std::exception& ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}

	std::cout << "server is shutting down..." << std::endl;

	return EXIT_SUCCESS;
}
