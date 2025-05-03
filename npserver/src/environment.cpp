// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "environment.h"
#include "bridge_udp.h"
#include "avr_virtual.h"
#include "protocol.h"
#include "history.h"
#include "config.h"
#include "avr_info/avr_info.h"
#include <npsys/network.h>
#include <npsys/avr_controller.h>
#include <nplib/utils/colored_cout.h>
#include "thread_pool.h"

namespace {
static std::unique_ptr<bridge_udp> br_udp;
//static npsys::virtual_pc_link_n v_pc_link;
static npsys::virtual_pc_link_serv_n v_pc_link_serv;
}

npsys::virtual_controllers_l::type_t* 
environment::find_virtual_controller_by_id(oid_t id) {
	auto founded = std::find_if(vc_list_.begin(), vc_list_.end(), [id](auto& n) {
		return n->id() == id;
	});
	return (founded != vc_list_.end() ? founded->get() : nullptr);
}

bool environment::store_virtual_controller(npsys::virtual_controllers_l::type_t* ptr) {
	auto founded = std::find_if(vc_list_.begin(), vc_list_.end(), [ptr](auto& n) {
		return n.get() == ptr;
	});
	return founded != vc_list_.end() ? founded->store() : false;
}

void environment::init() {
	npsys::server_n serv;
	auto ok = serv.fetch();
	if (!ok && serv.is_connection_loss()) {
		std::cerr << "database connection lost\n";
		std::abort();
	}

	if (!ok) serv.create();

//	memset(net_status_, 0x00, sizeof(net_status_));

	br_udp = std::make_unique<bridge_udp>(serv->ip_address, serv->port, serv->timeout);

	std::fill(br_map_.begin(), br_map_.end(), nullptr);

	if (!v_pc_link_serv.fetch()) {
		v_pc_link_serv.create(VirtualAvrPCLINK::Create());
	}
	br_map_[0] = v_pc_link_serv.get(); // pc? // virtual-pc-link

	br_map_[1] = br_udp.get(); // p-net bridge
	vc_list_.fetch_all_nodes();

	npsys::controllers_l controllers;
	controllers.fetch_all_nodes();
	for (auto& ctrl : controllers) {
		if (ctrl->firmware_status == npsys::CController::FIRMWARE_LOADED_DEVICE_ACTIVATED) {
			ctrl->AddToNetwork(ctrl.cast<npsys::device_n>());
		}
	}

	if (g_cfg.virtual_environment == 1) {
		v_pc_link_serv->run();
		std::cout << clr::color::green << "virtual pc link started." << clr::color::reset << std::endl;
	}
}

void environment::init_service_thread() {
	timer_.expires_from_now(boost::posix_time::seconds(0));
	timer_.async_wait(std::bind(&environment::sync_time, this, std::placeholders::_1));
}

bool environment::device_activated(npsys::device_n& device) {
	if (device->AddToNetwork(device)) {
		if (g_cfg.log_level > 2) std::cout << device->get_name() << " has been activated, dev_addr = " << device->dev_addr << std::endl;
		return true;
	}
	return false;
}

bool environment::device_deactivated(npsys::device_n& device) {
	if (br_map_[device->dev_addr] == nullptr) {
		std::cout << "device is not activated. skipping procedure..." << std::endl;
		return true;
	}

	if (g_cfg.log_level > 2) {
		std::cout << "begin device deactivation: dev_addr = " << device->dev_addr << std::endl;
	}

	proto->t_remove_device(device->dev_addr).get();

	br_map_[device->dev_addr] = nullptr;

	auto id = device.id();
	auto founded = std::find_if(vc_list_.begin(), vc_list_.end(), [id](auto& n) {
		return n->id() == id;
	});

	if (founded != vc_list_.end()) {
		(*founded)->stop();
		vc_list_.erase(founded, true);
		vc_list_.store();
	}

	if (g_cfg.log_level > 2) {
		std::cout << "device has been deactivated successfuly" << std::endl;
	}

	return true;
}

void environment::shutdown() {
	timer_.cancel();
	br_udp.reset();
}

void environment::sync_time(const boost::system::error_code& ec) noexcept {
	if (ec) {
		std::cout << "sync_time ec: " << ec.message() << std::endl;
		return;
	}
	
	timer_.expires_from_now(boost::posix_time::seconds(g_cfg.time_sync_period));
	timer_.async_wait(std::bind(&environment::sync_time, this, std::placeholders::_1));

	using namespace std::chrono_literals;

	static npsystem::types::Time lt;
	lt.sec.quality = VQ_GOOD;
	lt.min.quality = VQ_GOOD;
	lt.hour.quality = VQ_GOOD;
	lt.wday.quality = VQ_GOOD;
	lt.mday.quality = VQ_GOOD;
	lt.mon.quality = VQ_GOOD;
	lt.year.quality = VQ_GOOD;

	std::time_t t = std::time(nullptr);
	
	auto ptr = std::localtime(&t);
	if (!ptr) return;

	lt.sec.value = ptr->tm_sec;
	lt.min.value = ptr->tm_min;
	lt.hour.value = ptr->tm_hour;
	lt.wday.value = ptr->tm_wday + 1;
	lt.mday.value = ptr->tm_mday;
	lt.mon.value = ptr->tm_mon + 1;
	lt.year.value = ptr->tm_year - 100;


	uint8_t addr = 0xFF;
	for (auto& ri : runtime_info_) {
		addr++;
		if (!ri.active) continue;
		std::lock_guard<nplib::spinlock> lk(ri.mut);
		auto result = proto->t_add_task(protocol::write::t_write_block(addr, ri.tm_addr, &lt, sizeof(npsystem::types::Time))).get();
		if (result >= 0) {
			ri.accessable = true;
//			net_status_[addr] = 1;
		} else {
			ri.accessable = false;
//			net_status_[addr] = 0;
		}
	}

	if (g_cfg.log_level > 2) std::cout << "time sync sent." << std::endl;
}


namespace npsys {

bool CServer::AddToNetwork(npsys::device_n /* device */) {
	return true;
}

bool CAVRController::AddToNetwork(npsys::device_n device) {
	switch (controller_model) {
	//case npsys::hardware::Model::PC_LINK_VIRTUAL:
	//	environment::get_instance().br_map_[dev_addr] = br_udp.get();
	//	break;
	case npsys::hardware::Model::ATMEGA8:
	case npsys::hardware::Model::ATMEGA16:
		environment::get_instance().br_map_[dev_addr] = br_udp.get();
		if (g_cfg.log_level > 0) std::cout << "avr device \"" << name_ << "\" added" << std::endl;
		break;
	case npsys::hardware::Model::ATMEGA8_VIRTUAL:
	case npsys::hardware::Model::ATMEGA16_VIRTUAL: {
		auto ptr = environment::get_instance().find_virtual_controller_by_id(device.id());
		if (!ptr) {
			ptr = new VirtualAvrController(device.cast<npsys::controller_n_avr>());
			if (!environment::get_instance().vc_list_.add_value(ptr)) {
				std::cerr << "Error in database.\n";
				return false;
			}
			if (g_cfg.log_level > 0) std::cout << "virtual device \"" << name_ << "\" added" << std::endl;
			environment::get_instance().vc_list_.store();
		}
		environment::get_instance().br_map_[dev_addr] = v_pc_link_serv.get();
		if (g_cfg.log_level > 0) std::cout << "virtual device \"" << name_ << "\" was found" << std::endl;
		ptr->run();
		break;
	}
	default:
		assert(false);
		break;
	}

	auto& ri = environment::get_instance().runtime_info_[dev_addr];
	std::lock_guard<nplib::spinlock> lk(ri.mut);
	
	ri.accessable = false;
	ri.tm_addr = GetA_Time();
	ri.last_update = {};
	ri.active = true;
	
	return true;
}
bool CVirtualAVRPCLINK::AddToNetwork(npsys::device_n /* device */) {
	return true;
}


} // namespace npsys
