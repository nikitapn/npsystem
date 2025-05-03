// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <npsys/network.h>
#include <npsys/variable.h>
#include <npsys/fbdblock.h>
#include "history.h"
#include "config.h"

#include <npdb/constants.h>

#include <filesystem>
#include <fstream>
#include <source_location>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace bip = boost::interprocess;
namespace fs = std::filesystem;

class mdb_exception
	: public std::runtime_error {
public:
	int mdb_ec;

	mdb_exception(const char* message, int _mdb_ec)
		: std::runtime_error(message)
		, mdb_ec(_mdb_ec)
	{
	}
};

class HistoryImpl : public protocol::listener {
	//MDB_env* env_;
	//MDB_txn* txn_;
	//MDB_dbi dbi_;

	struct ParamMeta {
		oid_t oid;
		size_t pointer_offset;
		size_t types_size;
		std::array<std::tuple<uint64_t, int>, 8> types;
	};

	static constexpr size_t cv_point_size = 16;
	static constexpr size_t cv_max_dbs = 256;
	static constexpr size_t cv_max_temp_size = 4096;
	static constexpr size_t cv_window_size = cv_max_temp_size;

	class FileMap {
		bip::file_mapping file_;
		bip::mapped_region map_;

		HistoryImpl::ParamMeta* base_;
		void* current_;

		bool is_full() const noexcept {
			return (const char*)current_ > (const char*)base_ + cv_max_temp_size - cv_point_size;
		}

	public:
		void set(const std::array<uint8_t, 8>& data) {
			if (is_full()) {
			
			}
			
			uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch()
				).count();
			
			std::memcpy(current_, &time, 8);
			std::memcpy(static_cast<char*>(current_) + 8, data.data(), 8);
			current_ = static_cast<char*>(current_) + 16;
		}

		FileMap(const char* file_path, oid_t oid = 0)
			: file_(file_path, bip::read_write)
			, map_(file_, bip::read_write, 0, cv_max_temp_size)
		{
			base_ = static_cast<ParamMeta*>(map_.get_address());

			if (oid != 0) {
				base_->oid = oid;
				base_->pointer_offset = sizeof(ParamMeta);
				current_ = (char*)base_ + sizeof(ParamMeta);
			} else {
				current_ = (char*)base_ + base_->pointer_offset;
			}
		}

		~FileMap() {
			map_.flush();
			map_.~mapped_region();
			file_.~file_mapping();
		}
	};

	void E(int rc, const std::source_location location = std::source_location::current()) {
		(void)rc; (void)location;
		//if (rc == MDB_SUCCESS) return;

		// copying the error message placed in closed stack frame to std::string (win32 only)
		//std::string emsg(mdb_strerror(rc));

		//std::cerr << "file: "
		//	<< location.file_name() << "("
		//	<< location.line() << ":"
		//	<< location.column() << ") `"
		//	<< location.function_name() << "`: "
		//	<< emsg << '\n';

		//throw mdb_exception(emsg.c_str(), rc);
	}

	std::string oid_to_str(oid_t oid) {
		return std::string("id") + std::to_string(oid);
	}

	fs::path temp_directory_;
	npsys::history_l hlist_;
	std::unordered_map<oid_t, std::unique_ptr<FileMap>> files_;
	std::unordered_map<nps::var_handle, std::pair<npsys::fbd_slot_n, FileMap*>> ref_;

	FileMap* create_db(oid_t oid) {
		//E(mdb_txn_begin(env_, NULL, 0, &txn_));
		//E(mdb_dbi_open(txn_, oid_to_str(oid).c_str(), MDB_CREATE, &dbi_));
		//E(mdb_txn_commit(txn_));

		auto file = temp_directory_ / std::to_string(oid);

		{
			std::ofstream new_file(file, std::ios_base::binary);
			std::fill_n(std::ostreambuf_iterator<char>(new_file), cv_max_temp_size, '\0');
		}

		auto res = files_.emplace(
			oid,
			std::make_unique<FileMap>(reinterpret_cast<const char*>(file.u8string().c_str()), oid)
		);

		assert(res.second);

		auto& nf = res.first->second;

		return nf.get();
	}
public:
	void add_parameter(oid_t param_id) {
		assert(param_id >= odb::FIRST_ID);

//		boost::asio::post(strand_, [id, this]() {
//			npsys::fbd_slot_n slot(id);
//
//			do {
//				if (slot.fetch() == false) {
//					std::cerr << "slot does not exist... slot_id = " << id << std::endl;
//					break;
//				}
//
//				slot.advise(std::bind(&history::OnNodeEvent, this,
//					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//
//				auto& var = slot->var;
//				if (var.fetch() && var.loaded() && var->IsGood()) {
//					nps::DataDef_Array ar;
//					ar.length(1);
//
//					ar[0].dev_addr = var->GetDevAddr();
//					ar[0].mem_addr = var->GetAddr();
//					ar[0].type = var->GetType();
//					slot->online.set_type(var->GetType());
//
//					advise(ar);
//
//					ref_.emplace(
//						ar[0].h,
//						std::make_pair(slot, influxdb::Point(slot->id()))
//					);
//				}
//
//				history_list_.add_node(slot);
//				history_list_.store();
//			} while (0);
//			});
	}

	void remove_parameter(oid_t param_id) {
		assert(param_id >= odb::FIRST_ID);

//		boost::asio::post(strand_, [id, this]() {
//			npsys::fbd_slot_n slot(id);
//			do {
//				if (!slot.fetch()) {
//					std::cerr << "slot does not exist... slot_id = " << id << std::endl;
//					break;
//				}
//				auto founded = std::find_if(ref_.begin(), ref_.end(), [id](auto& pair) {
//					return pair.second.first.id() == id;
//					});
//
//				if (founded == ref_.end()) {
//					std::cerr << "advised item was not found... slot_id = " << id << std::endl;
//					break;
//				}
//				release_one(founded->first);
//			} while (0);
//
//			history_list_.erase(slot);
//			history_list_.store();
//			});
	}

	void OnNodeEvent(oid_t id, odb::NodeEvent event, std::span<uint8_t>) {
		switch (event) {
		case odb::NodeEvent::CHANGED:
			std::cout << "node changed: " << id << std::endl;
			break;
		case odb::NodeEvent::DELETED:
			std::cout << "node deleted: " << id << std::endl;
			break;
		default:
			assert(false);
			break;
		}
	}

	// called from io_context strand
	void OnDataChanged(const std::vector<nps::server_value>& ar) {
		for (auto& value : ar) {
			auto& x = ref_[value.h];
			if (value.s == nps::var_status::VST_DEVICE_RESPONDED) {
				x.second->set(value.data);
			}
		}
	}
	
	explicit HistoryImpl(const fs::path& db_path)
		: temp_directory_{db_path / "temp"}
	{
		if (!fs::exists(temp_directory_)) {
			fs::create_directories(temp_directory_);
		}

		//E(mdb_env_create(&env_));
		//E(mdb_env_set_maxdbs(env_, 128));
		//E(mdb_env_set_maxreaders(env_, 1));
		//E(mdb_env_set_mapsize(env_, 1024 * 1024 * 10));
		//E(mdb_env_open(env_, reinterpret_cast<const char*>(db_path.u8string().c_str()), MDB_FIXEDMAP /*|MDB_NOSYNC*/, 0664));

		for (auto const& file : fs::directory_iterator(temp_directory_)) {
			if (file.is_directory()) continue;

			auto file_path = file.path().u8string();
			auto str = reinterpret_cast<const char*>(file_path.c_str());

			oid_t oid;
			std::from_chars(str, str + file_path.length(), oid);

			files_.emplace(oid, std::make_unique<FileMap>(str));
		}

		npsys::controllers_l controllers;
		controllers.fetch_all_nodes();

		//	{
		//		history_list_.init();
		//		bool repaired = false;
		//		for (auto it = history_list_.begin(); it != history_list_.end();) {
		//			if (auto founded = std::find(std::next(it), history_list_.end(), *it); founded != history_list_.end()) {
		//				it = history_list_.erase(it);
		//				repaired = true;
		//			} else {
		//				it++;
		//			}
		//		}
		//		if (repaired) history_list_.store();
		//	}

		if (hlist_.fetch_all_nodes() == false) {
			hlist_.init();
			hlist_.store();
		}

		std::vector<npsys::fbd_slot_n> added_slots;
		for (auto& param : hlist_) {
			param.advise(std::bind(&HistoryImpl::OnNodeEvent, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			if (param->var.fetch()) {
				added_slots.push_back(param);
			}
		}

		if (added_slots.size() == 0) return;

		auto size = static_cast<uint32_t>(added_slots.size());

		std::vector<nps::flat::DataDef> ar(size);

		size = 0;

		for (auto& param : hlist_) {
			if (param->var.loaded()) {
				auto& var = param->var;
				auto dev_addr = var->GetDevAddr();
				if (dev_addr == -1) continue;
				ar[size].dev_addr = dev_addr;
				ar[size].mem_addr = var->GetAddr();
				ar[size].type = var->GetType();
				param->online.set_type(var->GetType());
				size++;
			}
		}

		{
			auto it = ar.begin();
			std::advance(it, size);
			ar.erase(it, ar.end());
		}

		std::vector<nps::var_handle> handles(size);
		{
			auto span = nprpc::flat::make_span(handles);
			t_advise(nprpc::flat::make_read_only_span(ar), span);
		}

		for (size_t ix = 0; ix < size; ++ix) {
			auto oid = added_slots[ix].id();
			FileMap* file_map = nullptr;
			{
				auto founded = files_.find(oid);
				if (founded == files_.end()) {
					file_map = create_db(oid);
				} else {
					file_map = founded->second.get();
				}
			}
			ref_.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(handles[ix]),
				std::forward_as_tuple(added_slots[ix], file_map)
			);
		}
		std::cout << size << " history items added for reading." << std::endl;
	}
};

History::History() {
	std::filesystem::path data_path(g_cfg.data_dir());
	data_path /= "history";
	impl_ = std::make_unique<HistoryImpl>(data_path);
}

History::~History() {
}

protocol::listener* History::get_listener() noexcept {
	assert(impl_);
	return impl_.get();
}

void History::add_parameter(oid_t param_id) {
	assert(impl_);
	impl_->add_parameter(param_id);
}

void History::remove_parameter(oid_t param_id) {
	assert(impl_);
	impl_->remove_parameter(param_id);
}
