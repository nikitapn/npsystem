// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <cassert>
#include <variant>
#include <sstream>

#include <npdb/db.h>
#include <nprpc_stub/nprpc_nameserver.hpp>

#include <nplib/utils/format.h>

#include <npsys/common_rpc.hpp>
#include <npsys/fat_data_callback.h>
#include <npsys/other/online_value.h>
#include <npsys/control_unit.h>
#include <npsys/algcat.h>
#include <npsys/system.h>

NPRPC_System* npsys_rpc;

void NPRPC_System::server_init() {
	auto nameserver = rpc->get_nameserver(nameserver_ip_);
	nprpc::Object* obj;
	if (!nameserver->Resolve("npsystem_server", obj)) {
		throw std::runtime_error("npserver could not be found");
	}

	server.reset(nprpc::narrow<nps::Server>(obj));

	assert(server->policy_lifespan() == nprpc::PoaPolicy::Lifespan::Persistent);
	
	try {
		server->Ping();
	} catch (...) {
		throw std::runtime_error("npserver is unreachable");
	}
}

void NPRPC_System::init(
	boost::asio::io_context& ioc,
	uint16_t /* port */,
	std::string_view nameserver_ip,
	std::string_view key_file_path,
	std::string_view module_name,
	int server_timeout,
	std::function<void()>&& notify)
{
	if (npsys_rpc) {
		std::cerr << "NPRPC_System has been previously initialized\n";
		std::abort();
	}

	npsys_rpc = new NPRPC_System();

	npsys_rpc->nameserver_ip_ = nameserver_ip;
	npsys_rpc->server_timeout_ = server_timeout;
	npsys_rpc->notify_ = std::move(notify);

	npsys_rpc->rpc = nprpc::RpcBuilder()
		.set_debug_level(nprpc::DebugLevel::DebugLevel_Critical)
		.set_listen_tcp_port(21500)
		.set_listen_http_port(21501)
		.build(ioc);

	npsys_rpc->callback_poa = nprpc::PoaBuilder(npsys_rpc->rpc)
		.with_max_objects(256)
		.with_lifespan(nprpc::PoaPolicy::Lifespan::Persistent)
		.build();

	auto nameserver = npsys_rpc->rpc->get_nameserver(nameserver_ip);
	odb::Database::init(nameserver.get(), npsys_rpc->callback_poa, key_file_path, module_name);

	npsys_rpc->corba_monitor_thread_ = std::thread(&NPRPC_System::monitor, npsys_rpc);
	
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

	if (corba_monitor_thread_.joinable())
		corba_monitor_thread_.join();

	delete this;
	npsys_rpc = nullptr;
}

void IFatDataCallBack::OnDataChanged(nprpc::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) {
	last_update_ = std::chrono::steady_clock::now();
	while (ready_for_callbacks_.load(std::memory_order_relaxed) == false);
	if (con_status_ == ConnectionStatus::advised_connected) {
		OnDataChangedImpl(a);
	}
}

bool IFatDataCallBack::Advise() noexcept {
	if (con_status_ != ConnectionStatus::unadvised)
		return true;
	if (!npsys_rpc->is_server_exist())
		return false;

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

	if (con_status_ == ConnectionStatus::unadvised)
		return;

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


namespace npsys {

// CControlUnit
void CControlUnit::SetScanPeriodMs(uint16_t _scan_period) {
	if (status_ == Status::status_loaded) {
		status_ = Status::status_not_actual;
	}
	npsys::access::rw(scan_period) = _scan_period;
}

// CAlgorithms
void CAlgorithms::CreateAlgorithmCat(std::string&& /* name */) noexcept {
	
}


CSystem::CSystem(std::string name) 
	: odb::systree_item<CSystem>(name) 
{
	npsys::controllers_l controllers;
	controllers.init();
	controllers.store();
}

} // namespace npsys




bool online_value::is_quality_not_good() const noexcept {
	if (val_.s == nps::var_status::VST_DEVICE_NOT_RESPONDED
		|| val_.s == nps::var_status::VST_UNKNOWN) {
		return true;
	}

	if (is_quality() == false) return false;
	
	switch (variable::GetClearType(type_)) {
	case type_t::NPT_BOOL:
		return to_Qbit().quality != VQ_GOOD;
	case type_t::NPT_U8:
		return to_Qu8().quality != VQ_GOOD;
	case type_t::NPT_I8:
		return to_Qi8().quality != VQ_GOOD;
	case type_t::NPT_U16:
		return to_Qu16().quality != VQ_GOOD;
	case type_t::NPT_I16:
		return to_Qi16().quality != VQ_GOOD;
	case type_t::NPT_U32:
		return to_Qu32().quality != VQ_GOOD;
	case type_t::NPT_I32:
		return to_Qi32().quality != VQ_GOOD;
	case type_t::NPT_F32:
		return to_Qflt().quality != VQ_GOOD;
	default:
		return true;
	};
}

void trim(std::string& s) {
	auto pos = s.rfind('.');
	if ((pos != std::string::npos) && (s.length() - pos > 4)) {
		s.erase(pos + 4);
	}
}

std::string online_value::to_string_impl(int /* precision */, bool show_quality) const noexcept {
	std::string result;
	using namespace npsystem::types;
	try
	{
		switch (val_.s)
		{
		case nps::var_status::VST_UNKNOWN:
			result = "unk";
			break;
		case nps::var_status::VST_DEVICE_NOT_RESPONDED:
			result = "nc";
			break;
		default:
			switch (variable::GetClearType(type_))
			{
			case type_t::NPT_BOOL:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QBIT& bt = to_Qbit();
						result = (bt.quality != VQ_GOOD) ? 'x' : (bt.value == NPSYSTEM_TRUE ? '1' : '0');
					} else {
						const QBIT& bt = to_Qbit();
						result = (bt.value == NPSYSTEM_TRUE ? "0b1, " : "0b0, ");
						result += (bt.quality == VQ_GOOD ? "0b1" : "0b0");
					}
				} else {
					result = to_bit().value == NPSYSTEM_TRUE ? '1' : '0';
				}
				break;
			case type_t::NPT_U8:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU8& val = to_Qu8();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU8& val = to_Qu8();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u8().value);
				}
				break;
			case type_t::NPT_I8:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI8& val = to_Qi8();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI8& val = to_Qi8();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i8().value);
				}
				break;
			case type_t::NPT_U16:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU16& val = to_Qu16();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU16& val = to_Qu16();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u16().value);
				}
				break;
			case type_t::NPT_I16:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI16& val = to_Qi16();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI16& val = to_Qi16();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i16().value);
				}
				break;
			case type_t::NPT_U32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QU32& val = to_Qu32();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QU32& val = to_Qu32();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_u32().value);
				}
				break;
			case type_t::NPT_I32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const QI32& val = to_Qi32();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
						}
					} else {
						const QI32& val = to_Qi32();
						result = nplib::format::to_hex(val.value) + ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					result = std::to_string(to_i32().value);
				}
				break;
			case type_t::NPT_F32:
				if (variable::IsQuality(type_)) {
					if (!show_quality) {
						const auto& val = to_Qflt();
						if (val.quality != VQ_GOOD) {
							result = 'x';
						} else {
							result = std::to_string(val.value);
							trim(result);
						}
					} else {
						const auto& val = to_Qflt();
						result = std::to_string(val.value);
						trim(result);
						result += ", " + std::to_string(static_cast<unsigned>(val.quality));
					}
				} else {
					const auto& val = to_flt();
					result = std::to_string(val.value);
					trim(result);
				}
				break;
			default:
				result = '?';
				break;
			}
			break;
		};
	}
	catch (std::bad_cast&)
	{
	}
	catch (...)
	{
	}
	return result;
}



#include <boost/tti/has_member_data.hpp>

BOOST_TTI_HAS_MEMBER_DATA(quality);

template<typename T>
inline void from_boolean_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += (val.value == NPSYSTEM_TRUE ? "true" : "false");
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';
	} else {
		out = "value=";
		out += (val.value == NPSYSTEM_TRUE ? "true" : "false");
	}
}

template<typename T>
inline void from_int_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += std::to_string(val.value) + 'i';
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';
	} else {
		out = "value=";
		out += std::to_string(val.value) + 'i';
	}
}

template<typename T>
inline void from_float_to_influx(const T& val, std::string& out) {
	if constexpr (has_member_data_quality<T, uint8_t>::value) {
		out = "value=";
		out += std::to_string(val.value);
		out += ",quality=";
		out += std::to_string(val.quality) + 'i';;
	} else {
		out = "value=";
		out += std::to_string(val.value);
	}
}

std::string online_value::to_influx_db() const noexcept {
	std::string result;
	try {
		switch (val_.s) {
		case nps::var_status::VST_UNKNOWN:
		case nps::var_status::VST_DEVICE_NOT_RESPONDED:
			break;
		default:
			switch (variable::GetClearType(type_)) {
			case type_t::NPT_BOOL:
				if (variable::IsQuality(type_)) {
					const auto& val = to_Qbit();
					from_boolean_to_influx(val, result);
				} else {
					const auto& val = to_bit();
					from_boolean_to_influx(val, result);
				}
				break;
			case type_t::NPT_U8:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qu8();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u8();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I8:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qi8();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i8();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_U16:
			if (variable::IsQuality(type_)) {
						const auto& val = to_Qu16();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u16();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I16:
			if (variable::IsQuality(type_)) {
						const auto& val = to_Qi16();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i16();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_U32:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qu32();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_u32();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_I32:
				if (variable::IsQuality(type_)) {
						const auto& val = to_Qi32();
						from_int_to_influx(val, result);
				} else {
						const auto& val = to_i32();
						from_int_to_influx(val, result);
				}
				break;
			case type_t::NPT_F32:
				if (variable::IsQuality(type_)) {
					const auto& val = to_Qflt();
					from_float_to_influx(val, result);
				} else {
					const auto& val = to_flt();
					from_float_to_influx(val, result);
				}
				break;
			default:
				break;
			}
			break;
		};
	}
	catch (std::bad_cast&)
	{
	}
	catch (...)
	{
	}
	return result;
}
