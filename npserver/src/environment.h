#pragma once

#include "bridge.h"
#include "config.h"
#include <npsys/header.h>
#include <nplib/utils/spinlock.h>
#include <mylib/singleton/singleton.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>
#include "thread_pool.h"
#include "server.h"
#include <npc/server.hpp>

namespace npsys {
class CServer;
class CAVRController;
}

class environment {
	friend npsys::CServer;
	friend npsys::CAVRController;

	struct RuntimeControllerData {
		bool active = false;
		bool accessable = false;
		nplib::spinlock mut;
		uint16_t tm_addr;
		std::chrono::steady_clock::time_point last_update;
	};

//	cbt::network_status net_status_;
	std::array<bridge*, 32> br_map_;
	std::array<RuntimeControllerData, 32> runtime_info_;
	npsys::virtual_controllers_l vc_list_;
	boost::asio::deadline_timer timer_;

	environment() noexcept
		: timer_(thread_pool::get_instance().ctx()) 
	{
	}
public:
	static environment& get_instance() noexcept {
		static environment instance;
		return instance;
	}
	void init();
	void init_service_thread();
	bridge* get_bridge(int addr) { return br_map_[addr]; }
	void validate_device(int dev_addr);
	bool device_activated(npsys::device_n& device);
	bool device_deactivated(npsys::device_n& device);
	npsys::virtual_controllers_l::type_t* find_virtual_controller_by_id(oid_t id);
	bool store_virtual_controller(npsys::virtual_controllers_l::type_t* ptr);
	void shutdown();
	void sync_time(const boost::system::error_code& ec) noexcept;
//	const  get_net_status() noexcept { return net_status_; }
//	static void m_remove_device(int dev_id, int dev_addr, std::string model);
};

inline void environment::validate_device(int dev_addr) {
	if ((dev_addr <= 1 || dev_addr >= 32) || (br_map_[dev_addr] == nullptr)) {
		if (g_cfg.log_level > 2) std::cerr << "cannot validate device with addr = " << dev_addr << '\n';
		throw nps::NPNetCommunicationError(S_COMMUNICATION_ERROR_DEVICE_NOT_EXIST);
	}
}