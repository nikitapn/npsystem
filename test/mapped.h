// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <array>
#include "dbtypes.h"

namespace npdb {

struct Chunk;

struct BlockInfo {
	block_size_t size;
	offset_t next_block; // next freed block
};

struct mapped_ptr_base {
	offset_t offset = invalid_offset;
	
	template<typename U>
	void* map(U& db) const noexcept {
		return db.get_ptr(offset);
	}

	bool operator == (const mapped_ptr_base& other) const noexcept { return offset == other.offset; }
	bool operator != (const mapped_ptr_base& other) const noexcept { return offset != other.offset; }
};

struct Chunk {
	mapped_ptr_base next;
};

constexpr size_t calc_chunk_size_cla(size_t chunk_size) noexcept {
	const size_t total_chunk_size = sizeof(Chunk) + chunk_size;
	return total_chunk_size + ((total_chunk_size & 63) ? size_t{64} - (total_chunk_size & size_t{63}) : 0);
}

template<typename T>
constexpr size_t calc_chunk_size_cla() noexcept {
	constexpr size_t total_chunk_size = sizeof(Chunk) + sizeof(T);
	return total_chunk_size + ((total_chunk_size & 63) ? 64 - (total_chunk_size & 63) : 0);
}

template<typename T, typename Derived>
class t_mapped_ptr
	: public mapped_ptr_base {
	Derived& derived() noexcept { return static_cast<Derived&>(*this); }
	const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }
protected:
	mutable Database* db_;
	mutable T* ptr_;
public:
	Database& db() noexcept { return *db_; }
	const Database& db() const noexcept { return *db_; }

	T* get() noexcept { derived().load_chunk(); return ptr_; }
	const T* get() const noexcept { derived().load_chunk(); return ptr_; }

	T* get_unsafe() noexcept { return ptr_; }
	const T* get_unsafe() const noexcept { return ptr_; }

	T& operator * () noexcept { derived().load_chunk(); return *ptr_; }
	const T& operator * () const noexcept { derived().load_chunk(); return *ptr_; }

	T* operator -> () noexcept { derived().load_chunk(); return ptr_; }
	const T* operator -> () const noexcept { derived().load_chunk(); return ptr_; }

	t_mapped_ptr(Database& db)
		: db_(&db)
		, ptr_(nullptr)
	{
	}

	t_mapped_ptr(offset_t _offset, Database& db)
		: mapped_ptr_base{_offset}
		, db_(&db)
	{
		derived().load_chunk();
	}

	t_mapped_ptr(const mapped_ptr_base& other, Database& db)
		: mapped_ptr_base(other)
		, db_(&db)
	{
		derived().load_chunk();
	}
};

template<typename T>
class mapped_ptr 
	: public t_mapped_ptr<T, mapped_ptr<T>>  {
	friend t_mapped_ptr<T, mapped_ptr<T>>;
	void load_chunk() const noexcept {
		assert(this->db_);
		auto chunk_ptr = (Chunk*)mapped_ptr_base::map(*this->db_);
		this->ptr_ = (T*)(chunk_ptr + 1);
	}
public:
	mapped_ptr(Database& db)
		: t_mapped_ptr<T, mapped_ptr<T>>(db)
	{
	}

	mapped_ptr(offset_t _offset, Database& db)
		: t_mapped_ptr<T, mapped_ptr<T>>(_offset, db)
	{
	}

	mapped_ptr(const mapped_ptr_base& other, Database& db)
		: t_mapped_ptr<T, mapped_ptr<T>>(other, db)
	{
	}
};

template<typename T>
class mapped_ptr_fixed 
	: public t_mapped_ptr<T, mapped_ptr_fixed<T>>  {
	friend t_mapped_ptr<T, mapped_ptr_fixed<T>>;
	void load_chunk() const noexcept {
		assert(this->db_);
		//static_assert(sizeof(T) % 64 == 0);
		this->ptr_= (T*)mapped_ptr_base::map(*this->db_);
	}
public:
	template<typename U>
	mapped_ptr_fixed<U> view(uint32_t _offset) {
		return mapped_ptr_fixed<U>{this->offset + _offset, *this->db_ };
	}
	
	mapped_ptr_fixed(Database& db)
		: t_mapped_ptr<T, mapped_ptr_fixed<T>>(db)
	{
	}

	mapped_ptr_fixed(offset_t _offset, Database& db)
		: t_mapped_ptr<T, mapped_ptr_fixed<T>>(_offset, db)
	{
	}

	mapped_ptr_fixed(const mapped_ptr_base& other, Database& db)
		: t_mapped_ptr<T, mapped_ptr_fixed<T>>(other, db)
	{
	}
};

struct AllocInfo {
	offset_t next_free_chunk;
	size_t payload_size;
};

__declspec(align(64))
struct TreeMeta {
	AllocInfo ai;
	mapped_ptr_base root;
};

std::ostream& operator<<(std::ostream& os, const TreeMeta& meta) {
	os << "payload_size: " << meta.ai.payload_size << ";root_offset: " << meta.root.offset;
	return os;
}

__declspec(align(64))
struct DatabaseMeta {
	char magic_thing[32];
	BlockInfo freed_block;
	offset_t free_space;
	TreeMeta main_tree;
};

template<typename T, typename DB, typename... Args>
mapped_ptr<T> make_mapped(DB& db, mapped_ptr_fixed<TreeMeta>& meta, Args&&... args) {
	auto [ptr, raw] = db.alloc_tree_node(meta);
	new (raw) T(std::forward<Args>(args)...);
	return mapped_ptr<T>(ptr, db);
}

} // namespace npdb

