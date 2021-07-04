// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <thread>
#include <atomic>
#include <cassert>
#include <functional>
#include <nplib/utils/thread_safe_set.h>
#include <npc/server.hpp>

class IFatDataCallBack;

class NPRPC_System {
	std::atomic_bool monitor_{true};
	bool server_exists_{false};
	int server_timeout_;
	std::string_view nameserver_ip_;
	std::thread corba_monitor_thread_;
	thread_safe_set<IFatDataCallBack*> data_callbacks_;
	std::function<void()> notify_;

	void server_init();
	void monitor() noexcept;

	NPRPC_System() = default;
	~NPRPC_System() = default;
public:	
	nprpc::Rpc* rpc;
	nprpc::Poa* callback_poa;
	nprpc::ObjectPtr<nps::Server> server;

	bool is_server_exist() const noexcept { return server_exists_; }
	int server_timeout() const noexcept { return server_timeout_; }

	void register_data_callback(IFatDataCallBack* data_callback) noexcept {
		data_callbacks_.insert(data_callback);
#ifdef _WIN32
		if (data_callbacks_.size() == 1) {
			SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED | ES_SYSTEM_REQUIRED);
		}
#endif
	}

	void unregister_data_callback(IFatDataCallBack* data_callback) noexcept {
		data_callbacks_.erase(data_callback);
#ifdef _WIN32
		if (data_callbacks_.size() == 0) {
			SetThreadExecutionState(ES_CONTINUOUS);
		}
#endif
	}
	
	void destroy() noexcept;
	
	static void init(
		boost::asio::io_context& ioc, 
		uint16_t port,
		std::string_view nameserver_ip,
		std::string_view key_file_path, 
		std::string_view module_name, 
		int server_timeout, 
		std::function<void()>&& notify);
};

extern NPRPC_System* npsys_rpc;

struct ObjectTimeout {
	nprpc::Object* obj;
	uint32_t old_timeout;
	
	ObjectTimeout(nprpc::Object* _obj, int timeout) 
		: obj{_obj} {
		assert(obj);
		old_timeout = obj->set_timeout(timeout);
	}

	~ObjectTimeout() {
		obj->set_timeout(old_timeout);
	}
};

