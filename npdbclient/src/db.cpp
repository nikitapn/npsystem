// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <fstream>
#include <filesystem>

#include <npdb/db.h>
#include <npc/db.hpp>
#include <nprpc/nprpc_nameserver.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

NPDB_IMPORT_EXPORT std::unordered_map<oid_t, std::weak_ptr<void>> odb::detail::table_objects::objects_;
NPDB_IMPORT_EXPORT std::unordered_map<oid_t, std::weak_ptr<void>> odb::detail::table_node_lists::objects_;

static constexpr oid_t alloc_id_size = 512;

static std::unique_ptr<odb::Database> instance_;

namespace odb {
class DatabaseImpl 
	: public Database
	, private npd::INodeCallback_Servant {

	struct Batch {
		int id;
		std::unordered_map<oid_t, std::pair<
			std::function<membuffer()>, 
			std::shared_ptr<detail::node_data>
		>> write_batch_nodes_;

		std::vector<std::shared_ptr<detail::node_data>> remove_batch_nodes_;
	};

	nprpc::ObjectPtr<npd::Database> db_;
	nprpc::ObjectId this_oid_;

	std::unordered_map<oid_t, advise_callback_t> callbacks_;

	std::vector<Batch> batch_;
	int batch_id_last_ = 0;
	oid_t free_id_next_;
	oid_t free_id_end_;
	bool key_allocated_ = false;
	std::filesystem::path key_file_;

	// POA_npd::INodeCallback
	NPDB_IMPORT_EXPORT
		virtual void OnNodeChanged(cbt1::oid_t id, ::nprpc::flat::Span<uint8_t> data) override {
		auto it = callbacks_.find(id);
		if (it != callbacks_.end()) {
			it->second(id, NodeEvent::CHANGED, {data.begin(), data.end()});
		}
	}
	NPDB_IMPORT_EXPORT
		virtual void OnNodeDeleted(::cbt1::oid_t id) override {
		auto it = callbacks_.find(id);
		if (it != callbacks_.end()) {
			it->second(id, NodeEvent::DELETED, {});
			callbacks_.erase(it);
		}
	}
public:
	NPDB_IMPORT_EXPORT
		virtual bool is_batch() { return !batch_.empty(); };

	NPDB_IMPORT_EXPORT
		virtual bool put_batch(
			std::function<membuffer()>&& fn, 
			std::shared_ptr<detail::node_data>&& node) 
	{
		auto& b = batch_.back();
		auto id = node->id;

//#ifdef _DEBUG
		auto repeat = b.write_batch_nodes_.find(id);
		if (repeat != b.write_batch_nodes_.end()) return true;
//#endif

		b.write_batch_nodes_.emplace(id, 
			std::make_pair(std::move(fn), std::move(node))
		);
		return true;
	}

	NPDB_IMPORT_EXPORT 
		virtual bool put(oid_t id, const membuffer& data, NodeError& e) {
		assert(id >= STATIC_ID_START);
		try {
			db_->put(id, {data.data(), data.data() + data.length()}, true);
			e = NodeError::E_NO_ERROR;
			return true;
		} catch (npd::DatabaseException&) {
			e = NodeError::E_STORE_FAILED;
		} catch (nprpc::ExceptionTimeout&) {
			e = NodeError::E_CONNECTION_LOSS;
		} catch (nprpc::Exception&) {
			e = NodeError::E_UNKNOWN_ERROR;
		}
		return false;
	}
	NPDB_IMPORT_EXPORT
		virtual REMOVE_RESULT remove(oid_t id, std::shared_ptr<detail::node_data>&& node) {
		if (batch_.size()) {
			auto& b = batch_.back();
			b.remove_batch_nodes_.push_back(std::move(node));
			return REMOVE_BATCH_MODE;
		}
		try {
			db_->remove(id);
			node->flags |= F_REMOVED;
			node->e = NodeError::E_NO_ERROR;
			return REMOVE_OK;
		} catch (npd::DatabaseException&) {
			node->e = NodeError::E_REMOVE_FAILED;
		} catch (nprpc::ExceptionTimeout&) {
			node->e = NodeError::E_CONNECTION_LOSS;
		} catch (nprpc::Exception&) {
			node->e = NodeError::E_UNKNOWN_ERROR;
		}
		return REMOVE_FAILED;
	}

	NPDB_IMPORT_EXPORT
		virtual bool get(oid_t id, bytestream& data, NodeError& e) {
		try {
			db_->get(id, data);
			e = NodeError::E_NO_ERROR;
			return true;
		} catch (npd::DatabaseException& ex) {
			if (ex.code == ldb_eNotFound) {
				e = NodeError::E_FETCH_FAILED_NOT_EXISTS;
			} else {
				e = NodeError::E_FETCH_FAILED;
			}
		} catch (nprpc::ExceptionTimeout&) {
			e = NodeError::E_CONNECTION_LOSS;
		} catch (nprpc::Exception&) {
			e = NodeError::E_UNKNOWN_ERROR;
		}
		return false;
	}

	NPDB_IMPORT_EXPORT
		virtual uint32_t get_n(const std::vector<oid_t>& ids, bytestream& data) {
		return db_->get_n(nprpc::flat::make_read_only_span(ids), data);
	}

	NPDB_IMPORT_EXPORT
		virtual void advise(oid_t id, advise_callback_t&& fn) {
		callbacks_[id] = std::move(fn);
		db_->advise(id, this_oid_);
	}

	NPDB_IMPORT_EXPORT
		virtual oid_t next_oid(oid_t amount_requested) {
		if (free_oid_amount() < amount_requested) {
			alloc_key_range(amount_requested > alloc_id_size ? amount_requested : alloc_id_size);
		}

		auto id = free_id_next_;
		free_id_next_ += amount_requested;
		key_allocated_ = true;

		if (batch_.empty()) write_key_file();
		
		return id;
	}

	size_t free_oid_amount() const noexcept { return free_id_end_ - free_id_next_; }

	void write_key_file() {
		if (!key_allocated_) return;
		std::ofstream ifs(key_file_, std::ios_base::binary);
		ifs.write((char*)&free_id_next_, sizeof(oid_t));
		ifs.write((char*)&free_id_end_, sizeof(oid_t));
		key_allocated_ = false;
	}

	void read_key_file() {
		std::ifstream ifs(key_file_, std::ios_base::binary);
		ifs.read((char*)&free_id_next_, sizeof(oid_t));
		ifs.read((char*)&free_id_end_, sizeof(oid_t));
	}

	void alloc_key_range(oid_t amount = alloc_id_size) {
		free_id_next_ = db_->next_oid(amount);
		free_id_end_ = free_id_next_ + amount;
		key_allocated_ = true;
		write_key_file();
	}

	DatabaseImpl(nprpc::Nameserver* nameserver, 
		nprpc::Poa* poa, std::filesystem::path keys_path, std::string_view key_file) {
	
		nprpc::Object* obj;
		nameserver->Resolve("npsystem_database", obj);
		db_.reset(nprpc::narrow<npd::Database>(obj));

		if (!db_) {
			throw std::runtime_error("could not get npsystem database");
		}

		db_->set_timeout(5000);

		this_oid_ = poa->activate_object(this);
		
		auto uid = db_->get_database_uuid();
		boost::uuids::uuid u;
		memcpy(&u, &uid[0], 16);
		
		auto s_uid = boost::lexical_cast<std::string>(u);

		key_file_ = std::move(keys_path);
		
		key_file_ /= key_file;
		key_file_ += "_" + s_uid + ".key";

		if (!std::filesystem::exists(key_file_)) {
			alloc_key_range();
		} else {
			read_key_file();
			if (db_->last_oid() < free_id_end_) {
				std::cerr << "cached last id is greater than last id in the current database.\n";
				alloc_key_range();
			}
		}

		std::cout << "key_file: " << key_file_ << std::endl;
	}
protected:
	NPDB_IMPORT_EXPORT 
		virtual int create_batch() noexcept {
		batch_.emplace_back();
		auto id = batch_id_last_++;
		batch_.back().id = id;
		return id;
	}

	NPDB_IMPORT_EXPORT
		virtual void remove_batch(int id) noexcept {
		auto it = std::find_if(batch_.begin(), batch_.end(), [id](auto& x) {
			return x.id == id;
		});
		if (it != batch_.end()) batch_.erase(it);
		write_key_file();
	}

	NPDB_IMPORT_EXPORT 
		virtual bool write_batch(int id) noexcept {

		auto it = std::find_if(batch_.begin(), batch_.end(), [id](auto& x) {
			return x.id == id;
		});
		
		assert(it != batch_.end());
		auto& b = *it;

		std::vector<npd::BatchOperation> data;

		for (auto& n : b.write_batch_nodes_) {
			membuffer buf = n.second.first();
			data.push_back({true, n.first, std::string((const char*)buf.data(), buf.length())});
		}

		for (auto& n : b.remove_batch_nodes_) {
			data.push_back({false, n->id, std::string()});
		}

		if (data.size() == 0) return true;
		
		try  {
			if (db_->exec_batch(nprpc::flat::make_read_only_span(data))) {
				for (auto& n : b.write_batch_nodes_) {
					if (n.second.second.use_count() != 1) n.second.second->after_put();
				}
				for (auto& n : b.remove_batch_nodes_) {
					if (n.use_count() != 1) n->after_remove();
				}
				batch_.erase(it);
				return true;
			}
		} catch (npd::DatabaseException& ex) {
			std::cout << "write_batch.npd::DatabaseException: " << ex.code << std::endl;
		} catch (nprpc::Exception& ex) {
			std::cout << "write_batch.CORBA::Exception: " << ex.what() << std::endl;
		}
		return false;
	}
};

NPDB_IMPORT_EXPORT
Database::~Database() {
}

NPDB_IMPORT_EXPORT
void Database::init(
	nprpc::Nameserver* nameserver, 
	nprpc::Poa* poa, 
	std::filesystem::path keys_path, 
	std::string_view key_file) {
	assert(!instance_);
	instance_.reset(new DatabaseImpl(nameserver, poa, keys_path, key_file));
}

NPDB_IMPORT_EXPORT
Database* Database::get() {
	assert(instance_);
	return instance_.get();
}

} // namespace odb