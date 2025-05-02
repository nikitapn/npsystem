// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef NPDB_HPP_
#define NPDB_HPP_

#include <iostream>
#include <optional>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <sstream>
#include <type_traits>
#include <filesystem>
#include <span>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/signals2/signal.hpp>

#include "../import_export.h"
#include "constants.h"
#include "traits.h"
#include "db_serialization.h"
#include "nodebuilder.h"
#include "memento.h"
#include "stream.h"


#ifdef DEBUG
#	define ASSERT_FETCH(node) assert(node.fetch())
#else
#	define ASSERT_FETCH(node) node.fetch()
#endif // DEBUG


// true if element has been erased
template<typename Vector>
bool VectorFastErase(Vector& v, typename Vector::value_type& elem) {
	constexpr auto invalid = std::numeric_limits<std::size_t>::max();
	std::size_t ix = invalid;
	std::size_t size = v.size();
	assert(size);
	for (std::size_t i = size - 1; i != invalid; --i) {
		if (v[i] == elem) {
			ix = i;
			break;
		}
	}
	if (ix == invalid) return false;
	if (ix != size - 1) v[ix] = std::move(v[size - 1]);
	v.pop_back();
	return true;
}

namespace nprpc {
class Rpc;
class Poa;
class Nameserver;
}

namespace odb {

using bytestream = std::vector<uint8_t>;
using advise_callback_t = std::function<void(oid_t, NodeEvent, std::span<uint8_t>)>;

class NodeHolder;

namespace detail {

struct node_data {
	oid_t id;
	uint32_t flags;
	NodeError e = NodeError::E_NO_ERROR;

	node_data() = default;
	node_data(oid_t _id, uint32_t _flags) : id(_id), flags(_flags) {}

	virtual void after_put() noexcept = 0;
	virtual void after_remove() noexcept = 0;
};
} // namespace detail

class Database {
	friend class Batch;
public:
	enum ErrorCode {
		ldb_eNotFound = 1,
		ldb_eCorruption = 2,
		ldb_eNotSupported = 3,
		ldb_eInvalidArgument = 4,
		ldb_eIOError = 5,
		eUnknown,
		eZeroAmount
	};

	NPDB_IMPORT_EXPORT
		static void init(
			nprpc::Nameserver* nameserver, 
			nprpc::Poa* poa, 
			std::filesystem::path keys_path, 
			std::string_view key_file);

	NPDB_IMPORT_EXPORT
		virtual ~Database();
	
	NPDB_IMPORT_EXPORT
		virtual bool is_batch() = 0;
	
	NPDB_IMPORT_EXPORT
		virtual bool put(oid_t id, const membuffer& data, NodeError& e) = 0;
	
	NPDB_IMPORT_EXPORT
		virtual bool put_batch(
			std::function<membuffer()>&& fn, 
			std::shared_ptr<detail::node_data>&&) = 0;
	
	enum REMOVE_RESULT {
		REMOVE_BATCH_MODE,
		REMOVE_OK,
		REMOVE_FAILED
	};
	
	NPDB_IMPORT_EXPORT
		virtual REMOVE_RESULT remove(oid_t id, std::shared_ptr<detail::node_data>&&) = 0;
	
	NPDB_IMPORT_EXPORT
		virtual bool get(oid_t id, bytestream& data, NodeError& e) = 0;
	
	NPDB_IMPORT_EXPORT
		virtual uint32_t get_n(const std::vector<oid_t>& ids, std::vector<uint8_t>& data) = 0;
	
	NPDB_IMPORT_EXPORT
		virtual void advise(oid_t id, advise_callback_t&& fn) = 0;
	
	NPDB_IMPORT_EXPORT
		virtual oid_t next_oid(oid_t amount) = 0;

	NPDB_IMPORT_EXPORT
		static Database* get();
protected:
	Database() = default;

	NPDB_IMPORT_EXPORT
		virtual int create_batch() noexcept = 0;
	
	NPDB_IMPORT_EXPORT
		virtual void remove_batch(int id) noexcept = 0;
	
	NPDB_IMPORT_EXPORT
		virtual bool write_batch(int id) noexcept = 0;
};

class Batch {
	const int id_;
public:
	bool exec() const noexcept {
		return Database::get()->write_batch(id_);
	}

	Batch() noexcept 
		: id_(Database::get()->create_batch())
	{
	}

	~Batch(){
		Database::get()->remove_batch(id_);
	}
};

namespace detail {

enum {
	UNIQUE_NODE,
	SHARED_NODE,
	WEAK_NODE
};

struct table_objects {
	NPDB_IMPORT_EXPORT static std::unordered_map<oid_t, std::weak_ptr<void>> objects_;
};

struct table_node_lists {
	NPDB_IMPORT_EXPORT static std::unordered_map<oid_t, std::weak_ptr<void>> objects_;
};

enum class LOCATION_RESULT {
	NOT_FOUND,
	REMOVED,
	FOUND,
	ERROR_EXPIRED,
};

template<typename T, bool static_oid, typename Table>
struct shared_policy_t {
	using table_t = Table;
	using policy = shared_policy_t<T, static_oid, Table>;

	struct node_data_t
		: node_data
		, std::enable_shared_from_this<node_data_t> {
		std::unique_ptr<T> obj;

		node_data_t() = default;
		node_data_t(oid_t _id, uint32_t _flags, T* _obj)
			: node_data(_id, _flags)
			, obj(_obj) {
		}
		virtual void after_put() noexcept final {
			if (!(this->flags & F_MAPPED)) {
				auto ptr = node_data_t::shared_from_this();
				policy::put(this->id, ptr);
			}
			this->flags |= F_REPLICATED;
			this->e = NodeError::E_NO_ERROR;

			if constexpr(has_modified_flag<T>::value) {
				this->obj->set_modified(false);
			}

			if constexpr(has_self_node_member<T>::value) {
				obj->self_node.from_weak(this->id,
					std::move(this->weak_from_this())
				);
			}
		}
		virtual void after_remove() noexcept final {
			if (this->flags & F_MAPPED) {
				policy::remove(this->id);
			}
			this->id = INVALID_ID;
			this->flags = F_REMOVED;
			this->e = NodeError::E_NO_ERROR;
		}
	};

	using smart_pointer = std::shared_ptr<node_data_t>;
	using weak_pointer = std::weak_ptr<node_data_t>;

	template<typename U>
	static smart_pointer make_pointer(oid_t id, uint32_t flags, U obj) {
		static_assert(std::is_pointer_v<U>, "obj is not a pointer");
		static_assert(std::is_base_of_v<T, std::remove_pointer_t<U>>, "cannot cast the pointer");
		return std::make_shared<node_data_t>(id, flags, obj);
	}

	static LOCATION_RESULT locate(oid_t id, smart_pointer& ptr) noexcept {
		assert(id >= STATIC_ID_START);
		auto it = Table::objects_.find(id);
		
		if (it == Table::objects_.end()) 
			return LOCATION_RESULT::NOT_FOUND;
		
		if (it->second.expired()) {
			Table::objects_.erase(it);
			return LOCATION_RESULT::NOT_FOUND;
		}

		auto locked = it->second.lock();
	  auto founded = std::static_pointer_cast<node_data_t>(locked);
		// auto founded = reinterpret_cast<weak_pointer&>(it->second).lock();
		
		if (founded->flags & F_REMOVED) { // should not happens
			return LOCATION_RESULT::REMOVED;
		}
		
		ptr = std::move(founded);
		return LOCATION_RESULT::FOUND;
	}

	static void put(oid_t id, smart_pointer& ptr) noexcept {
		assert(id >= STATIC_ID_START);
		auto it = Table::objects_.find(id);
		if (it == Table::objects_.end()) {
			Table::objects_[id] = ptr;
			ptr->flags |= F_MAPPED;
			return;
		}
		if (it->second.expired()) {
			Table::objects_[id] = ptr;
			ptr->flags |= F_MAPPED;
		} else {
			std::cout << "Error: node with id = " << id << " is not expired." << std::endl;
			assert(false); // error node is not expired. attempt to rewrite
		}
	}
	static void remove(oid_t id) noexcept {
		Table::objects_.erase(id);
	}
};

template<typename T, bool static_oid>
struct shared : shared_policy_t<T, static_oid, table_objects> {};

template<typename T, bool static_oid>
struct shared_node_list : shared_policy_t<T, static_oid, table_node_lists> {};

constexpr int archive_flags = boost::archive::archive_flags::no_header | boost::archive::archive_flags::no_codecvt
| boost::archive::archive_flags::no_tracking | boost::archive::archive_flags::no_xml_tag_checking;

template<typename T, typename NodeType, oid_t Default_Id, typename S>
class basic_node : public basic_node_abstract, public S {
public:
	using type_t = NodeType;
	using serialization_type = S;
protected:
	static membuffer to_membuffer(type_t* obj) {
		omembuf buff;
		{
			boost::archive::binary_oarchive oa(buff, archive_flags);
			if constexpr(type_t::has_common_interface()) {
				static_assert(std::is_abstract_v<typename type_t::serialize_as>, "Common interface should be abstact");
				oa << static_cast<typename type_t::serialize_as*>(obj);
			} else {
				oa << obj;
			}
		}
		return buff.transfer_to_membuffer();
	}
	// to do: wrap into try catch block
	static bool from_bytestream(const std::vector<uint8_t>& bstream, type_t*& obj) {
		boost::iostreams::array_source sink(
			(char*)&const_cast<const std::vector<uint8_t>&>(bstream)[0],
			bstream.size()
		);
		boost::iostreams::stream<boost::iostreams::array_source> stream(sink);
		boost::archive::binary_iarchive ia(stream, archive_flags);
		try {
			if constexpr(type_t::has_common_interface()) {
				static_assert(std::is_abstract_v<typename type_t::serialize_as>, "Common interface should be abstact");
				typename type_t::serialize_as* ptr;
				ia >> ptr;
				obj = static_cast<type_t*>(ptr);
			} else {
				type_t* ptr;
				ia >> ptr;
				obj = static_cast<type_t*>(ptr);
			}
			return true;
		} catch (std::exception& ex) {
			std::cerr << ex.what() << '\n';
		}
		return false;
	}
	T* this_() noexcept { return static_cast<T*>(this); }
	const T* this_() const noexcept { return static_cast<const T*>(this); }
public:
	static constexpr bool is_static_id() noexcept { return Default_Id != INVALID_ID; }
	static constexpr oid_t default_id() noexcept { return Default_Id; }

	bool is_replicated() const noexcept {
		return (this_()->get_flags() & F_REPLICATED) ? true : false;
	}
	bool is_connection_loss() const noexcept {
		return (this_()->get_e() == NodeError::E_CONNECTION_LOSS);
	}
	bool is_not_exist() const noexcept {
		return static_cast<bool>(this_()->get_flags() & F_REMOVED) ||
			this_()->get_e() == NodeError::E_FETCH_FAILED_NOT_EXISTS;
	}
	bool is_new_node() const noexcept {
		return this_()->id() == NEW_NODE_ID;
	}
	bool is_invalid_node() const noexcept {
		return this_()->id() == INVALID_ID;
	}
};

template <
	typename T,
	template <typename, bool> typename share_policy,
	oid_t Default_Id,
	typename S
>
class shared_node_t : public basic_node<
	shared_node_t<T, share_policy, Default_Id, S>, T, Default_Id, S> {
	
	template <typename, template <typename, bool> typename, oid_t, typename>
	friend class node_weak;
	friend class node_builder;
	friend NodeHolder;
public:
	using self = shared_node_t<T, share_policy, Default_Id, S>;
	using base = basic_node<shared_node_t<T, share_policy, Default_Id, S>, T, Default_Id, S>;
	using type_t = T;
	using policy = share_policy<T, Default_Id != INVALID_ID>;
	using smart_pointer = typename policy::smart_pointer;

	static constexpr auto node_type = SHARED_NODE;
	static constexpr auto is_shared() noexcept { return true; }
private:
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int/* version*/) const {
		if (data_) {
			if (data_->id == NEW_NODE_ID) {
				data_->id = Database::get()->next_oid(1);
				if constexpr(has_self_node_member<T>::value) {
					data_->obj->self_node = *this;
				}
			}
			ar << data_->id;
		} else {
			ar << tmp_id_;
		}

	}
	template<class Archive>
	void load(Archive & ar, const unsigned int/* version*/) {
		ar >> tmp_id_;
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		if constexpr(base::is_static_id()) {
			// nothing to serialize
		} else {
			boost::serialization::split_member(ar, *this, file_version);
		}
	}

	template<typename U>
	bool create_impl(U* obj, oid_t id) {
		assert(obj);
		assert(id >= STATIC_ID_START || id == NEW_NODE_ID);

		if (id == NEW_NODE_ID) id = Database::get()->next_oid(1);

		data_ = policy::make_pointer(id, 0, obj);

		if (Database::get()->is_batch()) {
			return Database::get()->put_batch(
				std::bind(&base::to_membuffer, data_->obj.get()),
				std::static_pointer_cast<node_data>(data_)
			);
		}

		if (Database::get()->put(id, base::to_membuffer(obj), data_->e) == false) return false;

		data_->flags |= F_REPLICATED;

		if constexpr(has_self_node_member<U>::value) {
			obj->self_node = *this;
		}

		data_->id = id;
		policy::put(id, data_);

		return true;
	}
public:
	shared_node_t()
		: tmp_id_(Default_Id) {}

	explicit shared_node_t(oid_t id) noexcept
		: tmp_id_(id) { // from pimpl node to type
		assert(id >= FIRST_ID);
		static_assert(!base::is_static_id(), "The node has static oid. Constructor is not allowed.");
	}
	
	oid_t id() const noexcept {
		if constexpr (base::is_static_id()) {
			return base::default_id();
		} else {
			return data_ ? data_->id : tmp_id_;
		}
	}

	uint32_t get_flags() const noexcept {
		return data_ ? data_->flags : tmp_flags_;
	}

	NodeError get_e() const noexcept {
		return data_ ? data_->e : tmp_e_;
	}

	template<typename U = T, typename... Args>
	bool create(Args&&... args) noexcept {
		auto ptr = new U(std::forward<Args>(args)...);
		bool ok = create_impl(ptr, base::is_static_id() ? Default_Id : NEW_NODE_ID);
		if (!ok) delete ptr;
		return ok;
	}

	template<typename U = T>
	bool create(U* ptr, bool remove_if_failed = true) noexcept {
		assert(ptr);
		bool ok = create_impl(ptr, base::is_static_id() ? Default_Id : NEW_NODE_ID);
		if (!ok && remove_if_failed) delete ptr;
		return ok;
	}

	template<typename U>
	bool create(oid_t id, U* ptr, bool remove_if_failed = true) noexcept {
		assert(ptr);
		bool ok = create_impl(ptr, id);
		if (!ok && remove_if_failed) delete ptr;
		return ok;
	}

	template<typename U = T>
	void create_locally_with_old_id(U* obj = nullptr) noexcept {
		assert(!data_);
		assert(tmp_id_ > STATIC_ID_START);
		data_ = policy::make_pointer(tmp_id_, 0, (obj ? obj : new U()));
		if constexpr(has_self_node_member<U>::value) {
			data_->obj->self_node = *this;
		}
		policy::put(tmp_id_, data_); // put
	}

	template<typename U = T>
	void create_locally_with_new_id(U* obj = nullptr) noexcept {
		assert(!data_);
		data_ = policy::make_pointer(NEW_NODE_ID, 0, (obj ? obj : new U()));
		if constexpr(has_self_node_member<U>::value) {
			data_->obj->self_node = *this;
		}
		// no need to put here !
	}

	template<typename U = T>
	void create_locally_with_id(oid_t id, U* obj = nullptr) noexcept {
		assert(!data_);
		assert(id >= FIRST_ID);

		static_assert(!base::is_static_id(), "The node has static oid.");

		data_ = policy::make_pointer(id, 0, (obj ? obj : new U()));
		if constexpr(has_self_node_member<U>::value) {
			data_->obj->self_node = *this;
		}
		policy::put(id, data_); // put
	}

	auto try_fetch_local() noexcept {
		if (tmp_id_ == INVALID_ID) return LOCATION_RESULT::NOT_FOUND;
		return policy::locate(tmp_id_, data_);
	}

	bool fetch() const noexcept {
		if (data_) return true;

		if (tmp_id_ == INVALID_ID) return false;
		
		auto loc_result = policy::locate(tmp_id_, data_);
		if (loc_result == LOCATION_RESULT::FOUND) return true;
		if (loc_result == LOCATION_RESULT::REMOVED) return false;

		auto self_id = id();

		bytestream stream;
		if (Database::get()->get(self_id, stream, tmp_e_) == false) return false;
		
		type_t* obj;
		if (base::from_bytestream(stream, obj) == false) {
			tmp_e_ = NodeError::E_ARCHIVE_CORRUPT;
			return false;
		}

		data_ = policy::make_pointer(self_id, F_REPLICATED, obj);
		data_->flags |= F_REPLICATED;
		policy::put(self_id, data_);
		
		if constexpr(has_modified_flag<T>::value) {
			obj->set_modified(false);
		}
		if constexpr(has_self_node_member<T>::value) {
			obj->self_node = *this;
		}
		return true;
	}

	static bool store_impl(smart_pointer& data) {
		if (!data) return false;
		
		if (data->flags & F_REMOVED) {
			std::cout << "attempt to store removed node" << std::endl;
			return false;
		}

		if (data->id == NEW_NODE_ID) {
			auto id = Database::get()->next_oid(1);
			data->id = id;
			if constexpr(has_self_node_member<T>::value) {
				data->obj->self_node.from_shared(id, data);
			}
		}

		if (Database::get()->is_batch()) {
			return Database::get()->put_batch(
				std::bind(&base::to_membuffer, data->obj.get()),
				std::static_pointer_cast<node_data>(data)
			);
		}

		if (Database::get()->put(data->id, base::to_membuffer(data->obj.get()), data->e) == false) return false;

		data->flags |= F_REPLICATED;

		if constexpr(has_modified_flag<T>::value) {
			data->obj->set_modified(false);
		}

		if (!(data->flags & F_MAPPED)) policy::put(data->id, data);

		return true;
	}

	static bool store_impl_copy_ptr(smart_pointer data) {
		return store_impl(data);
	}

	bool store() noexcept {
		return store_impl(data_);
	}

	membuffer serialize_to_membuffer() {
		return base::to_membuffer(data_->obj.get());
	}

	bool remove() noexcept {
		if (base::is_invalid_node()) return true;
		if (base::is_new_node()) {
			assert(data_);
			data_->id = INVALID_ID;
			data_->flags = F_REMOVED;
			data_->e = NodeError::E_NO_ERROR;
			if constexpr (has_observable_remove<T>::value) {
				static_cast<T*>(this)->sig_node_removed();
			}
			return true;
		}

		if (!loaded()) {
			auto ok = fetch();
			if (!ok && tmp_e_ == NodeError::E_FETCH_FAILED_NOT_EXISTS) {
				tmp_id_ = INVALID_ID;
				tmp_flags_ = F_REMOVED;
				tmp_e_ = NodeError::E_NO_ERROR;
				return true;
			}
			if (!ok) return false;
		} else {
			if constexpr (has_observable_remove<T>::value) {
				static_cast<T*>(this)->sig_node_removed();
			}
		}

		auto result = Database::get()->remove(id(), std::static_pointer_cast<node_data>(data_));

		if (result == Database::REMOVE_BATCH_MODE) return true;
		if (result == Database::REMOVE_OK) {
			if (data_->flags & F_MAPPED) {
				policy::remove(data_->id);
			}
			data_ = {};
			tmp_id_ = INVALID_ID;
			tmp_flags_ = F_REMOVED;
			tmp_e_ = NodeError::E_NO_ERROR;
		}
		return result == Database::REMOVE_OK;
	}

	bool advise(advise_callback_t&& fn) {
		Database::get()->advise(id(), std::move(fn));
		return true;
	}

	T* operator->() noexcept { return data_->obj.get(); }
	const T* operator->() const noexcept { return data_->obj.get(); }
	bool loaded() const noexcept { return (bool)data_; }
	auto use_count() const noexcept { return data_.use_count(); }
	
	template<class U>
	bool operator==(const U& other) const noexcept {
		static_assert(U::node_type == SHARED_NODE, "Types are incomparable.");
		
		auto this_id = id();
		auto other_id = other.id();
	
		assert(this_id != INVALID_ID && other_id != INVALID_ID);

		if (this_id == NEW_NODE_ID && other_id == NEW_NODE_ID) {
			return get() == other.get();
		} else if (this_id == NEW_NODE_ID || other_id == NEW_NODE_ID) {
			return false;
		} else {
			return this_id == other_id;
		}
	}

	template<class U>
	bool operator!=(const U& other) const noexcept {
		return !operator==(other);
	}

//	dangerous removed
//	bool operator < (const shared_node_t& other) const noexcept {
//		return id() < other.id();
//	}

	T* get() noexcept {
		assert(data_);
		return data_->obj.get();
	}

	const T* get() const noexcept {
		assert(data_);
		return data_->obj.get();
	}

	template<typename U>
	U cast() {
		static_assert(std::is_base_of_v<basic_node_abstract, U>, "Type U is not node type.");
		using cast_type_t = typename U::type_t;
		static_assert(
			std::is_base_of_v<type_t, cast_type_t> ||
			std::is_base_of_v<cast_type_t, type_t>,
			"Cannot cast to a different node type.");
		using cast_smart_ptr_type = typename U::smart_pointer::element_type;
		return node_builder::template create_after_cast<U>(tmp_id_, tmp_flags_, 
			std::reinterpret_pointer_cast<cast_smart_ptr_type>(data_));
	}

	template<typename U>
	void assign(const U& t) noexcept {
		static_assert(!base::is_static_id(), "The node has static oid. Assign is not allowed.");
		static_assert(
			std::is_base_of_v<type_t, typename U::type_t> ||
			std::is_base_of_v<typename U::type_t, type_t>,
			"Cannot assign from a different objects");
		data_.reset();
		tmp_id_ = t.id();
		tmp_flags_ = 0;
	}

	void reset() noexcept {
		static_assert(!base::is_static_id(),
			"reset is not available for static node");
		tmp_id_ = INVALID_ID;
		tmp_flags_ = 0;
		data_.reset();
	}

	void free() noexcept {
		if (!data_) return;
		tmp_id_ = data_->id;
		tmp_flags_ = 0;
		data_.reset();
	}
	operator std::shared_ptr<void> () {
		return data_;
	}
private:
	std::conditional_t<Default_Id != INVALID_ID, const oid_t, oid_t> tmp_id_;
	mutable uint32_t tmp_flags_ = 0;
	mutable NodeError tmp_e_ = NodeError::E_NO_ERROR;
	mutable smart_pointer data_;

	shared_node_t(smart_pointer&& data) noexcept
		: data_(data) {}

	shared_node_t(oid_t tmp_id, uint32_t tmp_flags, smart_pointer data) noexcept
		: tmp_id_(tmp_id)
		, tmp_flags_(tmp_flags)
		, data_(data) {}

	template<typename U = T>
	shared_node_t(U* obj) noexcept {
		static_assert(!base::is_static_id(), "This constructor is not allowed for static ID");

		data_ = policy::make_pointer(NEW_NODE_ID, 0, obj);
		if constexpr(has_self_node_member<U>::value) {
			obj->self_node = *this;
		}
	}
};

template<typename T>
class inplace_node_list : public MementoCommonSupport {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & data_;
		if constexpr(Archive::is_saving::value) {
			changed_ = false;
		}
		if constexpr(Archive::is_loading::value) {
			if constexpr (std::is_base_of_v<boost::archive::binary_special, Archive> &&
				std::is_base_of_v<special_serialization, node_t>) {
				// special serialization will be having loaded nodes already
			} else {
				nodes_fetched_ = false;
			}
		}
	}
public:
	static constexpr bool _inplace = true;
	using node_t = T;
	using type_t = typename node_t::type_t;
	using iterator = typename std::vector<node_t>::iterator;

	static constexpr auto has_common_interface() noexcept { return false; }

	[[maybe_unused]]
	size_t fetch_nodes() {
		if (nodes_fetched_ || data_.empty()) return data_.size();

		nodes_fetched_ = true;

		std::vector<oid_t> ids(data_.size());

		size_t size_requested = 0;
		for (auto it = data_.begin(); it != data_.end();) {
			if ((*it).loaded()) {
				// check if removed
				it++;
				continue;
			}
			LOCATION_RESULT result = (*it).try_fetch_local();
			if (result == LOCATION_RESULT::REMOVED) {
				it = data_.erase(it);
				continue;
			}
			if (result == LOCATION_RESULT::FOUND) {
				it++;
				continue;
			}
			ids[size_requested++] = (*it).id();
			it = data_.erase(it);
		}

		ids.resize(size_requested);

		if (size_requested == 0) return data_.size();

		bytestream bstream;
		auto size_recieved = Database::get()->get_n(ids, bstream);

#ifdef DEBUG
		if (size_recieved != size_requested) {
			std::cout << "Error in file: " << __FILE__ << "line: " << __LINE__ << '\n' << "class: " << typeid(inplace_node_list<T>).name() << " recived: " << size_recieved << " requested: " << size_requested << std::endl;
		}
#endif // DEBUG

//		assert(size_recieved == size_requested);
		if (size_recieved == 0) return 0;

		boost::iostreams::array_source sink((char*)&bstream[0], bstream.size());
		boost::iostreams::stream<boost::iostreams::array_source> stream(sink);
		boost::archive::binary_iarchive ia(stream, archive_flags);
		std::string value;

		for (uint32_t ix = 0; ix < size_recieved; ++ix)
		{
			try
			{
				oid_t id;
				type_t* ptr;

				ia >> id;
				ia >> value;

				boost::iostreams::array_source sink2(value.data(), value.length());
				boost::iostreams::stream<boost::iostreams::array_source> stream2(sink2);
				boost::archive::binary_iarchive ia2(stream2, archive_flags);

				if constexpr(type_t::has_common_interface()) {
					static_assert(std::is_abstract_v<typename type_t::serialize_as>, "Common interface should be abstact");
					typename type_t::serialize_as* ptr_as;
					ia2 >> ptr_as;
					ptr = static_cast<type_t*>(ptr_as);
				} else {
					ia2 >> ptr;
				}
				data_.push_back(node_builder::template create_assuming_there_wasnt_any<node_t>(id, ptr));
			} catch (boost::archive::archive_exception& ex) {
				std::cout << ex.what() << "\n"
					<< typeid(type_t).name() << std::endl;
			} catch (std::exception& ex) {
				std::cout << ex.what() << "\n"
					<< typeid(type_t).name() << std::endl;
			}
		}


		return data_.size();
	}
	std::size_t size() const noexcept { return data_.size(); }
	auto begin() noexcept { return data_.begin(); }
	auto rbegin() noexcept { return data_.rbegin(); }
	auto end() noexcept { return data_.end(); }
	auto rend() noexcept { return data_.rend(); }
	auto cbegin() const noexcept { return data_.cbegin(); }
	auto cend() const noexcept { return data_.cend(); }
	
	void push_back_node(node_t& n) noexcept {
		data_.push_back(n);
		changed_ = true;
	}

	void pop_back() {
		data_.pop_back();
		changed_ = true;
	}

	bool push_back_value(type_t* value) noexcept {
		node_t new_node;
		if (!new_node.create(value)) return false;
		data_.push_back(std::move(new_node));
		changed_ = true;
		return true;
	}

	node_t back() noexcept {
		return data_.back();
	}

	void clear() noexcept {
		changed_ = true;
		data_.clear();
	}

	iterator erase(iterator it) {
		changed_ = true;
		return data_.erase(it);
	}

	bool erase(node_t& n) {
		assert(n.id() != INVALID_ID);
		if (auto founded = find(n); founded != data_.end()) {
			changed_ = true;
			data_.erase(founded);
			return true;
		}
		return false;
	}

	bool fast_erase(node_t& n) noexcept {
		changed_ = true;
		return VectorFastErase(data_, n);
	}

	bool fast_erase(oid_t id) noexcept {
		node_t tmp(id);
		return fast_erase(tmp);
	}

	auto insert(iterator it, const node_t& n) noexcept {
		changed_ = true;
		return data_.insert(it, n);
	}

	bool is_changed() const noexcept { return changed_; }
	virtual std::unique_ptr<IMemento> CreateMemento_Common() noexcept {
		return MakeMemento(*this, data_);
	}
	iterator find(const node_t& n) {
		assert(n.id() != INVALID_ID);
		return std::find(data_.begin(), data_.end(), n);
	}
private:
	std::vector<node_t> data_;
	bool changed_ = false;
	bool nodes_fetched_ = true;
};

template<typename NodeType, oid_t default_id, typename S>
class node_list_base : public basic_list_abstract, public S {
public:
	static constexpr bool _inplace = false;
	using node_t = NodeType;
	using list_t = inplace_node_list<node_t>;
	using type_t = typename node_t::type_t;
	using node_iterator = typename std::vector<node_t>::iterator;
	using const_node_iterator = typename std::vector<node_t>::const_iterator;
	using vector_node = shared_node_t<list_t, detail::shared_node_list, default_id, S>;
	using self = node_list_base<NodeType, default_id, S>;
private:
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
		if constexpr (vector_node::is_static_id() == false)
		{
			if (n_.id() == INVALID_ID) {
				assert(n_.loaded() == false);
				n_.assign(vector_node(Database::get()->next_oid(1)));
			}
		}
		ar << n_;
	}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		ar >> n_;
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		if constexpr(default_id == INVALID_ID) {
			boost::serialization::split_member(ar, *this, file_version);
		}
		if constexpr(Archive::is_saving::value) {
			if (n_.loaded() && n_->is_changed()) {
				store();
			}
		}
	}
public:
	node_list_base() = default;

	node_list_base(const node_list_base& other) {
		assert(other.id() != INVALID_ID);

		n_ = other.n_;
	}

	node_list_base& operator=(const node_list_base& other) noexcept {
		assert(other.id() != INVALID_ID);

		n_ = other.n_;
		return *this;
	}

	node_iterator begin() noexcept {
		if (!n_.loaded()) return node_iterator();
		return n_->begin();
	}

	node_iterator end() noexcept {
		if (!n_.loaded()) return node_iterator();
		return n_->end();
	}

	const_node_iterator begin() const noexcept {
		if (!n_.loaded()) return const_node_iterator();
		return n_->cbegin();
	}

	const_node_iterator end() const noexcept {
		if (!n_.loaded()) return const_node_iterator();
		return n_->cend();
	}

	bool fetch_all_nodes() const noexcept {
		if (n_.id() == INVALID_ID) return true;
		if (n_.is_new_node()) return true;
		if (!n_.fetch()) return false;
		n_->fetch_nodes();

		assert(!n_.is_invalid_node());

		return true;
	}

	bool add_node(node_t& n) noexcept {
		if (!init()) return false;
		n_.get()->push_back_node(n);
		return true;
	}

	auto erase(node_iterator it, bool remove_node_from_db = false) noexcept {
		if (remove_node_from_db) it->remove();
		return n_->erase(it);
	}

	bool erase(node_t& n, bool remove_node_from_db = false) noexcept {
		assert(n.id() != INVALID_ID);

		if (!init()) return false;

		auto founded = find(n);
		if (founded == n_.get()->end()) return false;
		
		n_->erase(founded);
		if (remove_node_from_db) n.remove();

		return true;
	}

	bool fast_erase(node_t& n) noexcept {
		assert(n.id() != INVALID_ID);
		if (!init()) return false;
		return n_->fast_erase(n);
	}

	bool fast_erase(oid_t id) noexcept {
		assert(id != INVALID_ID);
		if (!init()) return false;
		return n_->fast_erase(id);
	}

	bool pop_back() noexcept {
		if (!init()) return false;
		n_->pop_back();
		return true;
	}

	bool store() noexcept {
		return n_.store();
	}

	bool loaded() const noexcept { return n_.loaded(); }
	
	bool is_replicated() const noexcept { return n_.is_replicated(); }

	oid_t id() const noexcept { return n_.id(); }
	
	void clear_list(bool with_nodes = false) {
		if (!n_.fetch()) return;
		if (with_nodes) {
			for (auto& n : *this) {
				n.remove();
			}
		}
		n_.get()->clear();
		n_.store();
	}
	
	void remove() {
		if (n_.loaded()) n_->clear();
		n_.remove();
		n_ = {};
	}
	
	size_t size() const noexcept {
		if (n_.is_not_exist() || n_.is_invalid_node()) return 0;
		assert(n_.loaded());
		return n_->size();
	}

	size_t get_index(const node_t& n) const noexcept {
		assert(n_.loaded());
		auto it = n_->find(n);
		if (it == n_->end()) return -1;
		return std::distance(n_->begin(), it);
	}

	node_t& operator[](size_t index) {
		assert(n_.loaded());
		assert(index < n_->size());
		if (index >= n_->size()) throw std::range_error("out of boundaries");
		auto it = n_->begin();
		std::advance(it, index);
		return (*it);
	}
	
	// call this before assign newly created list node (with invalid id) to other list node
	// for correct weak pointers creation
	bool init() noexcept {
		bool result = false;
		do {
			if (n_.loaded()) {
				result = true;
				break;
			}
			if constexpr (vector_node::is_static_id() == false) {
				if (n_.is_invalid_node()) { // for new unserialized list
					n_.create_locally_with_new_id();
					result = true;
					break;
				}
			}

			assert(n_.id() != INVALID_ID);

			if (n_.fetch()) {
				result = true;
				break;
			}

			if (n_.is_not_exist()) {
				n_.create_locally_with_old_id();
				result = true;
				break;
			}
		} while (0);

		assert(n_.id() != INVALID_ID); // database error

		return result;
	}
	vector_node& node() noexcept { return n_; }
protected:
	node_iterator find(const node_t& n) {
		assert(n.id() != INVALID_ID);
		return std::find(n_->begin(), n_->end(), n);
	}

	mutable vector_node n_;
};


template<typename Node, oid_t default_id, typename S, typename SFINAE = void>
class node_list;

template<typename Node, oid_t default_id, typename S>
class node_list<Node, default_id, S, std::enable_if_t<Node::node_type == UNIQUE_NODE>>
	: public node_list_base<Node, default_id, S> {
	friend boost::archive::binary_oarchive_special;
	friend boost::archive::binary_iarchive_special;
public:
	using base = node_list_base<Node, default_id, S>;
	using list_t = typename base::list_t;
	using type_t = typename base::type_t;

	bool add_value(type_t* ptr, bool remove_if_failed = true) noexcept {
		if (!base::init()) return false;
		if (base::n_.get()->push_back_value(ptr)) return true;
		if (remove_if_failed) delete ptr;
		return false;
	}
};

template<typename Node, oid_t default_id, typename S>
class node_list<Node, default_id, S, std::enable_if_t<Node::node_type == SHARED_NODE>>
	: public node_list_base<Node, default_id, S> {
	friend boost::archive::binary_oarchive_special;
	friend boost::archive::binary_iarchive_special;
public:
	using base = node_list_base<Node, default_id, S>;
	using list_t = typename base::list_t;
	using type_t = typename base::type_t;

	std::optional<Node> add_value(type_t* ptr, bool remove_if_failed = true) noexcept {
		if (base::init() &&
			base::n_->push_back_value(ptr)) {
			return base::n_->back();
		}
		if (remove_if_failed) delete ptr;
		return {};
	}
};

template<typename T, template <typename, bool> typename share_policy, oid_t default_id, typename S>
class node_weak {
public:
	using type_t = T;
	using node_t = shared_node_t<T, share_policy, default_id, S>;
	static constexpr auto node_type = WEAK_NODE;
private:
	template<typename, template <typename, bool> typename, oid_t, typename>
	friend class node_weak;

	using opaque_ptr = typename share_policy<void, default_id != INVALID_ID>::smart_pointer;
	static constexpr bool is_static_id() noexcept {
		return default_id != INVALID_ID;
	}
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int) const {
		if (id_ >= STATIC_ID_START) {
			ar << id_;
		} else {
			if (!ptr_.expired()) {
				auto ptr = std::reinterpret_pointer_cast<typename opaque_ptr::element_type>(ptr_.lock());
				// auto ptr = reinterpret_cast<opaque_ptr&>(ptr_.lock());
				assert(ptr->id != INVALID_ID);
				if (ptr->id == NEW_NODE_ID) {
					ptr->id = Database::get()->next_oid(1);
				}
				ar << ptr->id;
			} else {
				ar << id_;
			}
		}
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int) {
		ar >> id_;
		// assert(id_ >= STATIC_ID_START);
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		if constexpr(!is_static_id()) {
			boost::serialization::split_member(ar, *this, file_version);
		}
	}
public:
	node_weak() = default;

	template<class U>
	node_weak(const U& n) {
		assign(n);
	}

	template<class U>
	node_weak& operator=(const U& n) {
		assign(n);
		return *this;
	}

	oid_t id() const noexcept {
		if (!ptr_.expired()) {
			auto ptr = std::reinterpret_pointer_cast<typename opaque_ptr::element_type>(ptr_.lock());
			return ptr->id;
		}
		return id_;
	}

	void set_id_unsafe(oid_t id) noexcept {
		assert(is_invalid_id());
		assert(ptr_.use_count() == 0);
		id_ = id;
	}

	node_t fetch() const noexcept {
		if (auto locked = ptr_.lock()) {
			return node_t(std::move(locked));
		} else {
			assert(id_ >= STATIC_ID_START);
			node_t n(id_);
			bool ok = n.fetch();
			if (ok) ptr_ = n.data_;
			return n;
		}
	}
	template<class U>
	void from_weak(oid_t id, U&& ptr) {
		id_ = id;
		ptr_ = std::move(
			reinterpret_cast<typename node_t::policy::weak_pointer&>(ptr)
		);
	}
	template<class U>
	void from_shared(oid_t id, U& ptr) {
		id_ = id;
		ptr_ = reinterpret_cast<typename node_t::smart_pointer&>(ptr);
	}
	template<class U>
	bool operator==(const U& n) const noexcept {
		return id_ == n.id();
	}
	bool is_invalid_id() const noexcept {
		return id_ == INVALID_ID;
	}
private:
	oid_t id_ = default_id;
	mutable typename node_t::policy::weak_pointer ptr_;

	template<class U>
	std::enable_if_t<U::node_type != WEAK_NODE> assign(const U& n) {
		static_assert(
			std::is_base_of_v<type_t, typename U::type_t> ||
			std::is_base_of_v<typename U::type_t, type_t>,
			"Cannot assign from a different objects");
		assert(n.loaded());
		id_ = n.id();
		// TODO: change to shared_ptr pointer cast to avoid warning
		ptr_ = reinterpret_cast<typename node_t::smart_pointer&>(const_cast<U&>(n).data_);
	}
	template<class U>
	std::enable_if_t<U::node_type == WEAK_NODE> assign(const U& n) {
		static_assert(
			std::is_base_of_v<type_t, typename U::type_t> ||
			std::is_base_of_v<typename U::type_t, type_t>,
			"Cannot assign from a different objects");
		id_ = n.id_;
		// TODO: change to shared_ptr pointer cast to avoid warning
		ptr_ = reinterpret_cast<typename node_t::policy::weak_pointer&>(const_cast<U&>(n).ptr_);
	}
};

} // namespace detail

// node
template<typename T, oid_t default_id = INVALID_ID, typename S = basic_serialization>
using shared_node = detail::shared_node_t<T, detail::shared, default_id, S>;

template<typename T, oid_t default_id = INVALID_ID, typename S = basic_serialization>
using unique_node = detail::shared_node_t<T, detail::shared, default_id, S>;

// pimpl_node

template<typename T, oid_t default_id = INVALID_ID, typename S = basic_serialization>
using shared_node_weak = detail::node_weak<T, detail::shared, default_id, S>;

template<typename T, oid_t default_id = INVALID_ID, typename S = basic_serialization>
using unique_node_weak = detail::node_weak<T, detail::shared, default_id, S>;

template<typename T, oid_t default_id = INVALID_ID, typename S = basic_serialization>
using node_list = detail::node_list<T, default_id, S>;

template<typename N>
using inplace_node_list = detail::inplace_node_list<N>;

template<typename T, typename... Args>
inline T create_node(Args&&... args) noexcept {
	return detail::node_builder::template create_new<T>(new typename T::type_t(std::forward<Args>(args)...));
}

struct hash {
	template<class T>
	std::size_t operator()(T const & n) const {
		return n.id();
	}
};

template<typename Node>
using weak_node = std::conditional_t<Node::node_type == detail::SHARED_NODE,
	shared_node_weak<typename Node::type_t, Node::default_id(), typename Node::serialization_type>,
	unique_node_weak<typename Node::type_t, Node::default_id(), typename Node::serialization_type>>;

template<typename Node>
struct self_node_member {
	using self_node_t = weak_node<Node>;
	self_node_t self_node;
	oid_t id() const noexcept { return self_node.id(); }
};

class modified_flag {
public:
	bool is_modified() const noexcept { return modified_; }
	void set_modified(bool modified = true) { modified_ = modified; }
private:
	bool modified_ = true;
};


class systree_item_const_name {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name_;
	}
protected:
	std::string name_;
public:
	systree_item_const_name() = default;
	
	template<typename U>
	explicit systree_item_const_name(U&& name) 
		: name_(std::forward<U>(name)) {}

	const std::string& get_name() const noexcept {
		return name_;
	}
};

template<typename T>
class systree_item : public systree_item_const_name {
public:
	systree_item() = default;
	
	template<typename U>
	explicit systree_item(U&& name) 
		: systree_item_const_name(std::forward<U>(name)) {}
	
	template<typename U>
	void set_name(U&& name) noexcept {
		name_ = std::forward<U>(name);
		if constexpr(has_modified_flag<T>::value) {
			static_cast<T*>(this)->set_modified();
		}
	}
};

template<typename T>
class systree_item_observable : public systree_item<T> {
	using inherited = systree_item<T>;

	boost::signals2::signal<void(const std::string&)> signal_;

public:
	systree_item_observable() = default;
	
	template<typename U>
	explicit systree_item_observable(U&& name) 
		: inherited(std::forward<U>(name)) {}
	
	template<typename U>
	void set_name(U&& name) noexcept {
		inherited::set_name(std::forward<U>(name));
		signal_(inherited::name_);
	}
	auto observe_name(std::function<void(const std::string&)>&& fn) noexcept {
		return signal_.connect(fn);
	}
};

class id_support {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & id_;
	}
public:
	id_support() = default;
	explicit id_support(bool)
		: id_(odb::Database::get()->next_oid(1)) {}
	explicit id_support(const id_support&)
		: id_(odb::Database::get()->next_oid(1)) {}

	oid_t GetId() const noexcept { return id_; }
	bool operator == (const id_support& other) const noexcept {
		return id_ == other.id_;
	}
private:
	oid_t id_;
};

template<class T>
class object_list {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar & list_;
		using is_saving = typename Archive::is_saving;
		if constexpr(is_saving::value) {
			//	if (modified_) {
			list_.store();
			//		modified_ = false;
			//	}
		}
	}
	struct NodeData
		: common_interface<no_interface> {
		template<class Archive>
		void serialize(Archive & ar, const unsigned int file_version) {
			ar & data;
		}
		std::vector<std::unique_ptr<T>> data;
	};
public:
	using iterator = typename std::vector<std::unique_ptr<T>>::iterator;
	bool create() noexcept { return list_.create(); }
	bool fetch() const noexcept { return list_.fetch(); }
	auto begin() noexcept { return list_->data.begin(); }
	auto end() noexcept { return list_->data.end(); }
	void push_back(T&& t) noexcept {
		modified_ = true;
		list_->data.push_back(std::move(t));
	}
	bool remove() noexcept { return list_.remove(); }
	template<class... _Valty>
	T* emplace_back(_Valty&&... _Val) {
		modified_ = true;
		auto ptr = std::make_unique<T>(std::forward<_Valty>(_Val)...);
		auto raw_ptr = ptr.get();
		list_->data.push_back(std::move(ptr));
		return raw_ptr;
	}
	
	void erase(const T* val) noexcept {
		modified_ = true;
		auto id = val->GetId();
		auto founded = std::find_if(list_->data.begin(), list_->data.end(), [id](const auto& ptr) {
			return id == ptr->GetId();
		});
		if (founded != list_->data.end()) {
			list_->data.erase(founded);
		} else {
			assert(false);
		}
	}

	bool loaded() const noexcept { return list_.loaded(); }

	auto erase(iterator it) noexcept {
		modified_ = true;
		return list_->data.erase(it);
	}

	object_list<T> clone() const noexcept {
		object_list<T> copy;
		copy.list_.create_locally_with_new_id();
		copy.modified_ = true;
		for (auto const& e : list_->data) {
			copy.list_->data.push_back(e->clone());
		}
		return copy;
	}
	auto& node() noexcept { return list_; }
private:
	bool modified_ = false;
	mutable unique_node<NodeData> list_;
};


#ifdef DEBUG
inline void check_memory_leak() {
	std::stringstream ss;
	bool error = false;
	for (auto& n : odb::detail::table_objects::objects_) {
		if (!n.second.expired()) {
			error = true;
			auto ptr = reinterpret_cast<std::weak_ptr<odb::detail::basic_node_abstract>&>(n.second).lock();
			ss << "id: " << n.first << " use_count: " << n.second.use_count() - 1 << "\n"
				<< typeid(*ptr.get()).name() << ".\n";
		}
	}
	if (error) {
		std::cout << ss.str() << std::endl;
		MessageBoxA(NULL, "found nodes being used", NULL, MB_ICONWARNING);
	}
}
#endif

template <typename T, typename = void>
struct is_mutable_name : std::false_type {};
template <typename T>
struct is_mutable_name<T, std::void_t<decltype(std::declval<T>().set_name(""))>> : std::true_type {};

template <typename T, typename = void>
struct is_node_list : std::false_type {};
template <typename T>
struct is_node_list<T, std::void_t<typename T::list_t>> : std::true_type {};

template <typename T, typename = void>
struct is_sys_tree_item : std::false_type {};
template <typename T>
struct is_sys_tree_item<T, std::void_t<decltype(std::declval<T>().get_name())>> : std::true_type {};


} // namespace odb

/*
inline bool operator < (const odb::detail::node_type_mutable_id& l, const odb::detail::node_type_mutable_id& r) {
	return l.id() < r.id();
}
inline bool operator < (const odb::detail::node_type_const_id& l, const odb::detail::node_type_const_id& r) {
	return l.id() < r.id();
}
*/

#include <boost/functional/hash.hpp>

namespace std {

template <
	typename T,
	template <typename, bool> typename share_policy,
	oid_t Default_Id,
	typename S
>
struct hash<odb::detail::shared_node_t<T, share_policy, Default_Id, S>> {
	std::size_t operator()(odb::detail::shared_node_t<T, share_policy, Default_Id, S> const& n) const {
		auto id = n.id();
		assert(id >= odb::STATIC_ID_START);
		size_t val = 0;
		boost::hash_combine(val, static_cast<uint64_t>(id));
		return val;
	}
};

} // namespace std

#endif // NPDB_HPP_

