// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <thread>
#include <string_view>
#include <mutex>

#include <leveldb/db.h>
#include <leveldb/comparator.h>
#include <leveldb/write_batch.h>

#include <npdb/db.h>
#include <npdb/leveldb_utils.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <nprpc/nprpc_nameserver.hpp>
#include <npc/db.hpp>

#include "config.h"

#ifdef _WIN32
#	include "windows.h"
#	include "Shlobj_core.h"
#	include <nplib/utils/win_service.h>
#endif

npdbserver::Config g_cfg;

using namespace odb;
namespace logging = boost::log;

static nprpc::Poa* poa;

uint8_t get_leveldb_status_code(const leveldb::Status& s) noexcept {
	// OK status has a NULL state_.  Otherwise, state_ is a new[] array
	// of the following form:
	//    state_[0..3] == length of message
	//    state_[4]    == code
	//    state_[5..]  == message
	// enum Code {
	//   kOk = 0,
	//   kNotFound = 1,
	//   kCorruption = 2,
	//   kNotSupported = 3,
	//   kInvalidArgument = 4,
	//   kIOError = 5
	// };
	static_assert(sizeof(leveldb::Status) == 8, "leveldb::Status has changed in size");
	auto state = *reinterpret_cast<const char* const*>(&s);
	return state == nullptr ? 0 : state[4];
}

class uuid_class 
	: public boost::uuids::uuid {
public:
	uuid_class()
		: boost::uuids::uuid(boost::uuids::random_generator()())
	{
	}
	explicit uuid_class(boost::uuids::uuid const& u)
		: boost::uuids::uuid(u)
	{
	}
};


class Database_impl
	: public npd::IDatabase_Servant
{
	struct client_nodes {
		nprpc::ObjectPtr<npd::NodeCallback> client;
		std::unordered_set<oid_t> nodes;
		//
		client_nodes() = default;
		client_nodes(::npd::NodeCallback* _client) : client(_client) {}
	};

	class OidComparator : public leveldb::Comparator {
	public:
		// Three-way comparison function:
		//   if a < b: negative result
		//   if a > b: positive result
		//   else: zero result
		virtual int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
			oid_t a1 = *reinterpret_cast<const oid_t*>(a.data());
			oid_t b1 = *reinterpret_cast<const oid_t*>(b.data());
			if (a1 < b1) return -1;
			if (a1 > b1) return +1;
			return 0;
		}
		// Ignore the following methods for now:
		virtual const char* Name() const { return "OidComparator"; }
		virtual void FindShortestSeparator(std::string*, const leveldb::Slice&) const {}
		virtual void FindShortSuccessor(std::string*) const {}
	};

	void level_db_throw_if_error(const leveldb::Status& s) const {
		if (!s.ok()) {
			auto code = get_leveldb_status_code(s);
			assert(code >= 1 && code <= 5);
			throw ::npd::DatabaseException(code);
		}
	}

	oid_t next_oid_impl(oid_t amount) {
		if (!amount) throw ::npd::DatabaseException(odb::Database::ErrorCode::eZeroAmount);

		std::lock_guard<std::mutex> lk{id_mut_};

		oid_t id = last_id_;
		last_id_ += amount;

		leveldb::WriteOptions wo;
		wo.sync = true;

		auto s = db_->Put(wo, create_slice(&COUNTER_ID), create_slice(&last_id_));
		level_db_throw_if_error(s);

		return id;
	}

	std::unique_ptr<leveldb::DB> db_;
	std::unique_ptr<OidComparator> oid_cmp_;
	std::mutex id_mut_;
	oid_t last_id_;
	std::vector<client_nodes> advised_clients_;
public:
	bool open(std::string path) noexcept {
		leveldb::DB* db_ptr;
		leveldb::Options options;
		options.comparator = oid_cmp_.get();
		auto s = leveldb::DB::Open(options, path, &db_ptr);
		if (!s.ok()) {
			options.create_if_missing = true;
			s = leveldb::DB::Open(options, path, &db_ptr);
			if (!s.ok()) {
				BOOST_LOG_TRIVIAL(error) << s.ToString();
				return false;
			} else {
				uuid_class uid;
				s = db_ptr->Put(leveldb::WriteOptions(), create_slice(&DB_GUID_ID), leveldb::Slice((char*)uid.data, 16));
				if (!s.ok()) return false;
			}
		}


		db_.reset(db_ptr);

		std::string str;

		s = db_->Get(leveldb::ReadOptions(), create_slice(&COUNTER_ID), &str);

		if (s.IsNotFound()) {
			last_id_ = FIRST_ID;
			s = db_->Put(leveldb::WriteOptions(), create_slice(&COUNTER_ID), create_slice(&last_id_));
			if (!s.ok()) return false;
		} else {
			last_id_ = read_oid(str);
		}

		return true;
	}

	virtual std::string get_database_name() {
		return g_cfg.db_name;
	}

	virtual cbt1::uuid get_database_uuid() {
		std::string value;
		auto s = db_->Get(leveldb::ReadOptions(), create_slice(&DB_GUID_ID), &value);

		level_db_throw_if_error(s);
		assert(value.size() == 16);

		cbt1::uuid uuid;
		std::memcpy(&uuid[0], value.data(), 16);

		return uuid;
	}

	virtual cbt1::oid_t next_oid(cbt1::oid_t amount) {
		return next_oid_impl(amount);
	}

	virtual cbt1::oid_t last_oid() {
		return last_id_;
	}

	virtual cbt1::oid_t create(::flat::Span<uint8_t> data, bool sync) {
		oid_t id = next_oid_impl(1);
		BOOST_LOG_TRIVIAL(trace) << "created new id: " << id;
		auto options = leveldb::WriteOptions();
		options.sync = sync;

		auto s = db_->Put(options, create_slice(&id), leveldb::Slice((const char*)data.data(), data.size()));

		level_db_throw_if_error(s);

		return id;
	}

	virtual cbt1::oid_t put(cbt1::oid_t id, ::flat::Span<uint8_t> data, bool sync) {
		auto options = leveldb::WriteOptions();
		options.sync = sync;

		auto s = db_->Put(options, create_slice(&id), leveldb::Slice((const char*)data.data(), data.size()));

		level_db_throw_if_error(s);

		for (auto& client : advised_clients_) {
			if (client.nodes.find(id) != client.nodes.end()) {
				client.client->OnNodeChanged(id, flat::make_read_only_span(data));
			};
		}
		return id;
	}

	virtual bool exec_batch(::flat::Span_ref<npd::flat::BatchOperation, npd::flat::BatchOperation_Direct> data) {
		leveldb::WriteBatch batch;
		std::string upd_nodes, del_nodes;

		for (auto x : data) {
			const auto key = create_slice(&x.id());
			if (x.create_or_update() == false) {
				del_nodes += std::to_string(x.id()) + ' ';
				batch.Delete(key);
			} else {
				upd_nodes += std::to_string(x.id()) + ' ';
				auto span = x.data();
				batch.Put(key, leveldb::Slice(static_cast<const char*>(span.data()), span.size()));
			}
		}

		auto options = leveldb::WriteOptions();
		options.sync = true;

		auto s = db_->Write(options, &batch);
		
		if (s.ok() == false) {
			std::cerr << "exec_batch failed: " << s.ToString() << '\n';
			return false;
		}

		std::cout
			<< "exec_batch:\n"
			<< "  deleted_nodes: " << del_nodes << '\n'
			<< "  updated_nodes: " << upd_nodes << std::endl;
		
/*
		for (auto& ni : t.info) {
			auto span = flat::make_read_only_span(ni.data);
			for (auto& client : advised_clients_) {
				if (client.nodes.find(ni.id) != client.nodes.end()) {
					if (ni.state == leveldb::KeyTraverse::NODE_UPDATED) {
						client.client->OnNodeChanged(ni.id, span);
					} else {
						client.client->OnNodeDeleted(ni.id);
					}
				}
			}
		}
*/
		return true;
	}

	virtual bool get(cbt1::oid_t id, /*out*/ ::flat::Vector_Direct1<uint8_t> data) {
		std::string tmp;
		auto s = db_->Get(leveldb::ReadOptions(), create_slice(&id), &tmp);
		level_db_throw_if_error(s);

		data.length(tmp.length());
		data() = std::string_view(tmp);

		return true;
	}

	virtual uint64_t get_n(::flat::Span<uint64_t> ids, /*out*/ ::flat::Vector_Direct1<uint8_t> data) {
		//	auto now = std::chrono::high_resolution_clock::now();
		size_t size = 0;
		omembuf buf(1024);
		boost::archive::binary_oarchive oa(buf, boost::archive::archive_flags::no_header);
		auto options = leveldb::ReadOptions();
		std::string value;
		for (auto id : ids) {
			auto s = db_->Get(options, create_slice(&id), &value);
			if (s.ok()) {
				oa << id;
				oa << value;
				//	oa.save_binary(value.c_str(), value.size());
				++size;
			}
		}

		//	std::chrono::duration<double, std::milli> d = std::chrono::high_resolution_clock::now() - now;
		//	std::cout << "time: " << d.count() << " ms." << std::endl;
		data.length(buf.length());
		std::memcpy(data().data(), buf.c_str(), buf.length());

		return size;
	}

	virtual void remove(cbt1::oid_t id) {
		auto options = leveldb::WriteOptions();
		level_db_throw_if_error(
			db_->Delete(options, create_slice(&id))
		);
		BOOST_LOG_TRIVIAL(trace) << "node has been deleted. id: " << id;
		for (auto& client : advised_clients_) {
			if (client.nodes.find(id) != client.nodes.end()) {
				client.client->OnNodeDeleted(id);
			};
		}
	}

	virtual void advise(cbt1::oid_t id, nprpc::Object* client) {
		auto it = std::find_if(advised_clients_.begin(), advised_clients_.end(), [client](client_nodes& c) {
			return client->_data().object_id == c.client->_data().object_id;
			});

		if (it == advised_clients_.end()) {
			client->add_ref();
			advised_clients_.emplace_back(nprpc::narrow<npd::NodeCallback>(client));
			it = std::prev(advised_clients_.end());
		} else {
			BOOST_LOG_TRIVIAL(warning) << "client is already advised for id " << id;
		}

		BOOST_LOG_TRIVIAL(trace) << "node with id " << id << " advised...";
		(*it).nodes.insert(id);
	}

	Database_impl()
		: oid_cmp_(new OidComparator())
	{
	}

	Database_impl(const Database_impl&) = delete;
	Database_impl& operator=(const Database_impl&) = delete;
};

template<typename T>
static int get_ini_value(boost::property_tree::ptree& pt, const std::string& path, T& value) {
	try {
		value = pt.get<T>(path);
		return 0;
	} catch (boost::property_tree::ptree_bad_data& ex) {
		std::cerr << "bad data: " << ex.data<std::string>() << '\n';
		throw;
	} catch (std::exception&) {
		//
	}
	return -1;
}

boost::asio::io_context ioc;

/// Allow to close cleanly with ctrl + c
void sigintHandler(int sig_num) {
	ioc.stop();
}

#ifdef _WIN32
int start(int argc, char** argv, Service::ready_t* ready = nullptr) {
#else 
int start(int argc, char** argv) {
#endif
	g_cfg.load("npdb");
	std::cout << g_cfg << std::endl;

	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::trace
	);

	Database_impl db_impl;

	if (!db_impl.open(g_cfg.db_name)) {
		std::cerr << "cannot open database\n";
		return -1;
	}

	try
	{
		auto wg = boost::asio::make_work_guard(ioc);
		auto ioc_thread = std::thread([]() { ioc.run(); });
		//std::thread([]() {ioc.run();}).detach();
		
		nprpc::set_debug_level(nprpc::DebugLevel_Critical);
		auto rpc = nprpc::init(ioc, 23255);

		poa = rpc->create_poa(1, {
			std::make_unique<nprpc::Policy_Lifespan>(nprpc::Policy_Lifespan::Persistent).get()
			});

		auto nameserver = rpc->get_nameserver(g_cfg.nameserver_ip);
		nameserver->Bind(poa->activate_object(&db_impl), "npsystem_database");

		rpc->start();

#ifdef _WIN32
		if (ready) (*ready)();
#endif

		ioc_thread.join();

		std::cout << "npdbserver: Returned from ioc.run()." << std::endl;
		rpc->destroy();

		return 0;
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC::Exception: " << ex.what() << '\n';
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
	}

	return -1;
}

int main(int argc, char** argv) {
#ifdef _WIN32
	if (Service::IsServiceMode()) {
		Service service("npdbserver");
		service.Start(start, std::bind(sigintHandler, 0));
	} else if (argc > 1) {
		if (strcmp(argv[1], "install") == 0) {
			std::cout << "Installing npdbserver as a service..." << std::endl;
			Service service("npdbserver");
			service.SvcInstall();
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
