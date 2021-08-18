// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <npsys/corba.h>
#include <npsys/fat_data_callback.h>
#include <npdb/db.h>
#include <nprpc/nprpc_nameserver.hpp>

NPRPC_System* npsys_rpc;

void NPRPC_System::server_init() {
	auto nameserver = rpc->get_nameserver(nameserver_ip_);
	nprpc::Object* obj;
	if (!nameserver->Resolve("npsystem_server", obj)) {
		throw std::runtime_error("npserver could not be found");
	}
	server.reset(nprpc::narrow<nps::Server>(obj));
	server->add_ref();

	assert(server->policy_lifespan() == nprpc::Policy_Lifespan::Persistent);
	
	try {
		server->Ping();
	} catch (...) {
		throw std::runtime_error("npserver is unreachable");
	}
}

void NPRPC_System::init(
	boost::asio::io_context& ioc,
	uint16_t port,
	std::string_view nameserver_ip,
	std::string_view key_file_path,
	std::string_view module_name,
	int server_timeout,
	std::function<void()>&& notify) {
	if (npsys_rpc) {
		std::cerr << "NPRPC_System has been previously initialized\n";
		std::abort();
	}

	npsys_rpc = new NPRPC_System();

	npsys_rpc->nameserver_ip_ = nameserver_ip;
	npsys_rpc->server_timeout_ = server_timeout;
	npsys_rpc->notify_ = std::move(notify);

	nprpc::Config rpc_cfg;
	rpc_cfg.debug_level = nprpc::DebugLevel::DebugLevel_Critical;
	rpc_cfg.port = 21500;

	npsys_rpc->rpc = nprpc::init(ioc, std::move(rpc_cfg));
	npsys_rpc->callback_poa = npsys_rpc->rpc->create_poa(256, 
		{ std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent).get() });
	
	auto nameserver = npsys_rpc->rpc->get_nameserver(nameserver_ip);
	odb::Database::init(nameserver.get(), npsys_rpc->callback_poa, key_file_path, module_name);

	npsys_rpc->corba_monitor_thread_ = std::thread(&NPRPC_System::monitor, npsys_rpc);
	
	npsys_rpc->rpc->start();
	npsys_rpc->server_init();
}

void NPRPC_System::monitor() noexcept {
	using namespace std::chrono_literals;
	bool prev = server_exists_;
	while (monitor_.load(std::memory_order_relaxed)) {
		try {
			if (!server_exists_) {
				server_init();
			} else {
				//server->Ping();
				thread_safe_for_each(data_callbacks_, [&](auto& callback) {
					callback->Check();
					});
			}
			server_exists_ = true;
		} catch (nprpc::Exception& ex) {
			server_exists_ = false;
			std::cerr << "NPRPC Exception: " << ex.what() << '\n';
		} catch (std::exception& ex) {
			server_exists_ = false;
			std::cerr << "Exception: " << ex.what() << '\n';
		}

		if (prev != server_exists_) {
			thread_safe_for_each(data_callbacks_, [&](auto& callback) {
				callback->Check();
				});
			notify_();
		}

		prev = server_exists_;
		std::this_thread::sleep_for(2s);
	}
}

void NPRPC_System::destroy() noexcept {
	monitor_ = false;

	if (corba_monitor_thread_.joinable()) {
		corba_monitor_thread_.join();
	}

	delete this;
	npsys_rpc = nullptr;
}

void IFatDataCallBack::OnDataChanged(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) {
	last_update_ = std::chrono::steady_clock::now();
	while (ready_for_callbacks_.load(std::memory_order_relaxed) == false);
	if (con_status_ == ConnectionStatus::advised_connected) {
		OnDataChangedImpl(a);
	}
}

bool IFatDataCallBack::Advise() noexcept {
	if (con_status_ != ConnectionStatus::unadvised) return true;
	if (!npsys_rpc->is_server_exist()) return false;

	ready_for_callbacks_ = false;

	try {
		if (AdviseImpl() != 0) {
			con_status_ = ConnectionStatus::advised_connected;
			ready_for_callbacks_ = true;
			npsys_rpc->register_data_callback(this);
			return true;
		}
		return false;
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Exception: " << ex.what() << '\n';
	}

	return false;
}

void IFatDataCallBack::UnAdvise() noexcept {
	if (con_status_ == ConnectionStatus::unadvised) return;
	ready_for_callbacks_ = false;
	try {
		item_manager_.reset();
	} catch (nprpc::Exception& ex) {
		std::cerr << "Failed to do UnAdvise. Exception: " << ex.what() << '\n';
	}
	npsys_rpc->unregister_data_callback(this);
	con_status_ = ConnectionStatus::unadvised;
	ready_for_callbacks_ = true;
}

bool IFatDataCallBack::IsAdvised() const noexcept {
	return con_status_ != ConnectionStatus::unadvised;
}

void IFatDataCallBack::EventConnectionLost() {
	npsys_rpc->callback_poa->deactivate_object(oid());
	con_status_ = ConnectionStatus::advised_not_connected;
	OnConnectionLost();
}

void IFatDataCallBack::Check() noexcept {
	while (ready_for_callbacks_.load(std::memory_order_relaxed) == false);
	if (con_status_ == ConnectionStatus::unadvised) return;

	if (!npsys_rpc->is_server_exist() && con_status_ == ConnectionStatus::advised_connected) {
		EventConnectionLost();
	} else if (con_status_ == ConnectionStatus::advised_not_connected) {
		try {
			AdviseImpl();
			con_status_ = ConnectionStatus::advised_not_connected;
		} catch (nprpc::Exception&/* ex*/) {
			//std::cerr << "NPRPC Exception: " << ex.what() << '\n';
		}
	} else {
		auto now = std::chrono::steady_clock::now();
		auto sec_passed = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_);
		if (sec_passed.count() > 3) {
			last_update_ = now;
			try {
				item_manager_->Ping();
			} catch (nprpc::ExceptionObjectNotExist&) {
				item_manager_.force_reset();
				EventConnectionLost();
			} catch (nprpc::Exception&) {
				EventConnectionLost();
			}
		}
	}
}