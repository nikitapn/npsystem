// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "protocol.h"
#include "history.h"
#include "itemmgr.h"
#include "bridge.h"
#include "config.h"
#include "avr_virtual.h"
#include "thread_pool.h"
#include <npdb/db.h>
#include <npsys/network.h>

#ifdef _WIN32
#	include "windows.h"
# include <conio.h>
#	include "Shlobj_core.h"
# include <nplib/utils/win_service.h>
#endif

#include <signal.h>
#include <filesystem>
#include <nprpc/nprpc_nameserver.hpp>
#include <npc/server.hpp>

npserver::Config g_cfg;

static nprpc::Poa* server1_poa;
static nprpc::Poa* item_manager_poa;

namespace nps {
class Server_ServantImpl
	: public IServer_Servant
{
	template<class F>
	int process_cmd(F&& fn) {
		int n = g_cfg.cmd_n_try, code;
		do {
			code = proto->t_add_task(std::ref(fn)).get();
			if (code >= 0) return code;
		} while (--n);
		if (g_cfg.log_level > 2) {
			std::cerr << "command failed: code = " << code << '\n';
		}
		throw nps::NPNetCommunicationError(code);
	}
public:
	virtual void Ping() {}

	virtual void CreateItemManager(nprpc::detail::flat::ObjectId_Direct im) {
		auto item_manager = new ItemManagerImpl();
		auto oid = item_manager_poa->activate_object(item_manager);
		nprpc::assign_to_out(oid, im);
	}

	virtual void SendRawData(::flat::Span<uint8_t> data) {
		proto->t_add_task(protocol::t_send_raw(data.data(), static_cast<uint8_t>(data.size())));
	}

	virtual void Write_1(uint8_t dev_addr, uint16_t mem_addr, uint8_t bit, uint8_t value) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_1(dev_addr, mem_addr, bit, value));
	}

	virtual void Write_q1(uint8_t dev_addr, uint16_t mem_addr, uint8_t bit, uint8_t value, uint8_t qvalue) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_1_q(dev_addr, mem_addr, bit, value, qvalue));
	}

	virtual void Write_8(uint8_t dev_addr, uint16_t mem_addr, uint8_t value) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_8(dev_addr, mem_addr, value));
	}

	virtual void Write_q8(uint8_t dev_addr, uint16_t mem_addr, uint8_t value, uint8_t qvalue) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_8(dev_addr, mem_addr, value, 1, qvalue));
	}

	virtual void Write_16(uint8_t dev_addr, uint16_t mem_addr, uint16_t value) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_16(dev_addr, mem_addr, value));
	}

	virtual void Write_q16(uint8_t dev_addr, uint16_t mem_addr, uint16_t value, uint8_t qvalue) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_16(dev_addr, mem_addr, value, 1, qvalue));
	}

	virtual void Write_32(uint8_t dev_addr, uint16_t mem_addr, uint32_t value) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_32(dev_addr, mem_addr, value));
	}

	virtual void Write_q32(uint8_t dev_addr, uint16_t mem_addr, uint32_t value, uint8_t qvalue) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_32(dev_addr, mem_addr, value, 1, qvalue));
	}

	virtual void WriteBlock(uint8_t dev_addr, uint16_t mem_addr, ::flat::Span<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::write::t_write_block(dev_addr, mem_addr, data.data(), static_cast<uint8_t>(data.size())));
	}

	virtual void ReadByte(uint8_t dev_addr, uint16_t addr, uint8_t& value) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::read::t_read_byte(dev_addr, addr, value));
	}

	virtual void ReadBlock(uint8_t dev_addr, uint16_t addr, uint8_t len, /*out*/::flat::Vector_Direct1<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::read::t_read_memory(dev_addr, addr, len, data));
	}

	virtual bool AVR_StopAlgorithm(uint8_t dev_addr, uint16_t alg_addr) {
		environment::get_instance().validate_device(dev_addr);
		auto code = process_cmd(protocol::avr5::t_stop_algorithm(dev_addr, alg_addr));
		return (code == 0 ? true : false);
	}

	virtual void AVR_ReinitIO(uint8_t dev_addr) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_reinit_io(dev_addr));
	}

	virtual void AVR_SendRemoteData(uint8_t dev_addr, uint16_t page_num, ::flat::Span<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_update_remote_data(dev_addr, page_num, data.data(), data.size()));
	}

	virtual void AVR_SendPage(uint8_t dev_addr, uint8_t page_num, ::flat::Span<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_send_page(dev_addr, page_num, data.data(), data.size()));
	}

	virtual void AVR_RemoveAlgorithm(uint8_t dev_addr, uint16_t alg_addr) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_remove_alg(dev_addr, alg_addr));
	}

	virtual void AVR_ReplaceAlgorithm(uint8_t dev_addr, uint16_t old_addr, uint16_t new_addr) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_replace_alg(dev_addr, old_addr, new_addr));
	}

	virtual void AVR_WriteEeprom(uint8_t dev_addr, uint16_t mem_addr, ::flat::Span<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_write_eeprom(dev_addr, mem_addr, data.data(), data.size()));
	}

	virtual void AVR_WriteTwiTable(uint8_t dev_addr, uint8_t page_num, ::flat::Span<uint8_t> data) {
		environment::get_instance().validate_device(dev_addr);
		process_cmd(protocol::avr5::t_write_twi_table(dev_addr, page_num, data.data(), data.size()));
	}

	virtual void AVR_V_GetFlash(cbt1::oid_t device_id, /*out*/::flat::Vector_Direct1<uint16_t> data) {
		auto virt = environment::get_instance().find_virtual_controller_by_id(device_id);
		auto v = dynamic_cast<VirtualAvrController*>(virt);
		if (v == nullptr) throw nps::NPNetCommunicationError(S_COMMUNICATION_ERROR_DEVICE_NOT_EXIST);

		const auto& flash = v->avr_->GetFlash();
		const auto size = flash.size();

		data.length(size);
		auto span = data();

		for (int i = 0; i < size; ++i) {
			span[i] = flash[i].instruction;
		}
	}

	virtual bool AVR_V_StoreFlash(cbt1::oid_t device_id) {
		auto virt = environment::get_instance().find_virtual_controller_by_id(device_id);
		auto v = dynamic_cast<VirtualAvrController*>(virt);
		if (v == nullptr) throw nps::NPNetCommunicationError(S_COMMUNICATION_ERROR_DEVICE_NOT_EXIST);
		auto res = environment::get_instance().store_virtual_controller(v);
		if (res) {
			std::cout << "flash saved for controller with addr: "
				<< v->ctrl->dev_addr << std::endl;
		} else {
			std::cout << "failed to store virtual controller with addr: "
				<< v->ctrl->dev_addr << std::endl;
		}
		return res;
	}

	virtual bool Notify_DeviceActivated(cbt1::oid_t device_id) {
		npsys::device_n device(device_id);
		if (device.fetch()) {
			return environment::get_instance().device_activated(device);
		}
		return false;
	}

	virtual bool Notify_DeviceDeactivated(cbt1::oid_t device_id) {
		npsys::device_n device(device_id);
		if (device.fetch()) {
			environment::get_instance().validate_device(device->dev_addr);
			return environment::get_instance().device_deactivated(device);
		}
		return false;
	}

	virtual void Notify_ParameterRemoved(cbt1::oid_t param_id) {
	}

	virtual void Notify_TypeOrVariableChanged(cbt1::oid_t param_id) {
	}

	virtual void History_AddParameter(cbt1::oid_t param_id) {
		// hist->AddParameter(param_id);
	}

	virtual void History_RemoveParameter(cbt1::oid_t param_id) {
		// hist->RemoveParameter(param_id);
	}

	virtual void GetNetworkStatus(/*out*/::flat::Vector_Direct1<uint8_t> network_status) {
	}
};

}

//////////////////////////////////////////////////////////////////////
std::unique_ptr<protocol::protocol_service> proto;
//std::unique_ptr<history> hist;

void sigint_handler(int sig_num) {
	std::cout << "npserver is shutting down..." << std::endl;
	try {
		proto->stop();
		std::cout << "protocol stopped()." << std::endl;

		thread_pool::get_instance().stop();
		std::cout << "thread pool stopped." << std::endl;
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC::Exception: " << ex.what() << '\n';
	} catch (...) {

	}
}

#ifdef _WIN32
void watch_x() {
	while (true) {
		if (_getch() == 'x') {
			sigint_handler(0);
			break;
		}
	}
}
#endif

#ifdef _WIN32
int start(int argc, char** argv, Service::ready_t* ready = nullptr) {
#else 
int start(int argc, char** argv) {
#endif

#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif
	signal(SIGINT, sigint_handler);
#ifdef _WIN32
	std::thread(&watch_x).detach();
#endif
	g_cfg.load("npserver");
	std::cout << g_cfg << std::endl;

	if (g_cfg.log_level > 2) {
		std::cout << "main_thread_id: " << std::this_thread::get_id() << std::endl;
	}

#ifdef _WIN32
	avrinfo::AVRInfo::SetDataPath(nplib::config::GetExecutableRootPath() / "data");
#else 
	avrinfo::AVRInfo::SetDataPath(g_cfg.data_dir / "data");
#endif

	if (!std::filesystem::exists(g_cfg.data_dir)) {
		std::cerr << "data directory not found: " << g_cfg.data_dir << '\n';
		return -1;
	}

	try {
		auto keypath = g_cfg.data_dir;
		keypath /= "keys";

		nps::Server_ServantImpl server_servant1;

		nprpc::set_debug_level(nprpc::DebugLevel_Critical);
		auto rpc = nprpc::init(thread_pool::get_instance().ctx(), 21000);

		server1_poa = rpc->create_poa(2, {
			std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent).get()
			});

		item_manager_poa = rpc->create_poa(512, {
			std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Transient).get()
			});

		auto nameserver = rpc->get_nameserver(g_cfg.nameserver_ip);
		odb::Database::init(nameserver.get(), server1_poa, keypath, "npserver");
		nameserver->Bind(server1_poa->activate_object(&server_servant1), "npsystem_server");

		rpc->start();

		environment::get_instance().init();
		proto = std::make_unique<protocol::protocol_service>();
		//			hist = std::make_unique<history>();
		//			hist->Init();
		environment::get_instance().init_service_thread();

#ifdef _WIN32
		if (ready) (*ready)();
#endif

		thread_pool::get_instance().ctx().run();
		std::cout << "Returned from io_contex.run()." << std::endl;
		//		item_mgr_pman->deactivate(true, true);
		//		server_pman->deactivate(true, true);

		environment::get_instance().shutdown();
		std::cout << "environment::get_instance().shutdown()." << std::endl;

		return 0;
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC::Exception: " << ex.what() << '\n';
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
	} catch (...) {
		std::cerr << "unknown exception\n";
	}

	thread_pool::get_instance().stop();

	return -1;
}

int main(int argc, char** argv) {
#ifdef _WIN32
	if (Service::IsServiceMode()) {
		Service service("npserver");
		service.Start(start, std::bind(sigint_handler, 0));
	} else if (argc > 1) {
		if (strcmp(argv[1], "install") == 0) {
			std::cout << "Installing npserver as a service..." << std::endl;
			Service service("npserver");
			service.SvcInstall(TEXT("npdbserver\0\0"));
	} else {
			std::cerr << "unknown argument...\n";
			return -1;
		}
} else {
		return start(argc, argv);
	}
	return 0;
#else
	return start(argc, argv);
#endif
}