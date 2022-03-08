// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <string_view>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "lru_cache.h"
#include "btree.h"

namespace npdb {

static constexpr size_t max_window_size = 1 * 0x100000;
//static constexpr size_t max_window_size = 8192;

namespace bip = boost::interprocess;
namespace fs = std::filesystem;

struct mapped_region {
	uint32_t index;
	bip::mapped_region region;

	bool operator < (const mapped_region& val) const noexcept { return index < val.index; }
	bool operator == (const mapped_region& val) const noexcept { return index == val.index; }
	bool operator == (const uint32_t val) const noexcept { return index == val; }

	mapped_region(uint32_t idx, const bip::file_mapping& m_file)
		: index{idx}
		, region{m_file, bip::read_write, idx * max_window_size}
	{
	}
};
} // namespace npdb

namespace std {
template<>
struct hash<npdb::mapped_region> {
	size_t operator()(const npdb::mapped_region& val) const noexcept {
		return std::hash<uint32_t>{}(val.index);
	}
};
} // namespace std

namespace npdb {

namespace bip = boost::interprocess;
namespace fs = std::filesystem;

struct Regions {
	LRUCache<uint32_t, mapped_region, 2> lst;
	bip::file_mapping mapped_file;

	static offset_t relative_offset(offset_t offset) noexcept {
		return offset % max_window_size;
	}

	static uint64_t region_index(offset_t offset) noexcept {
		return offset / max_window_size;
	}

	void* get_ptr(offset_t offset) {
		auto idx = region_index(offset);
		if (auto r = lst.get(idx); r) return (char*)r->region.get_address() + relative_offset(offset);
		return (char*)lst.emplace(idx, idx, mapped_file).region.get_address() + relative_offset(offset);
	}
};

class BlockIterator {
	Regions& regions_;
	offset_t offset_;
	size_t blocks_count_;
	block_size_t block_size_;
	void* ptr_;
public:
	bool is_done() const noexcept { return blocks_count_ == 0; }
	offset_t offset() const noexcept { return offset_; }
	void* operator*() noexcept { return ptr_; }

	BlockIterator& operator++() {
		assert(blocks_count_);
		auto next_region = Regions::region_index(offset_) + 1;
		if (offset_ + block_size_ * 2 > next_region * max_window_size) {
			offset_ = next_region * max_window_size;
			ptr_ = regions_.get_ptr(offset_);
		} else {
			ptr_ = (char*)ptr_ + block_size_;
			offset_ += block_size_;
		}
		--blocks_count_;
		return *this;
	}

	offset_t next_offset() const noexcept { 
		if (blocks_count_ == 1) return invalid_offset;
		auto const next_region = Regions::region_index(offset_) + 1;
		return
			offset_ + 2 * block_size_ > next_region * max_window_size
			? next_region * max_window_size
			: offset_ + block_size_;
	}

	BlockIterator(Regions& regions, offset_t offset, size_t blocks_count, block_size_t block_size) 
		: regions_(regions)
		, offset_(offset)
		, blocks_count_(blocks_count)
		, block_size_(block_size)
	{
		assert(blocks_count_);
		blocks_count_ -= 1;

		auto next_region = Regions::region_index(offset_) + 1;
		if (offset_ + block_size_ > next_region * max_window_size) {
			offset_ = next_region * max_window_size;
		}

		ptr_ = regions_.get_ptr(offset);
	}
};

class Database {
	static constexpr char cv_magic_thing[] = "npbtree_v.0.1";
	
	using MainTree = BTree<uint64_t, TreeMeta, 25>;
	
	fs::path file_path_;
	std::string u8_file_path_;
	size_t file_size_ = 0;
	
	Regions regions_;
	mapped_ptr_fixed<DatabaseMeta> meta_;

	void resize_file(size_t added_regions_number) {
		file_size_ += max_window_size * added_regions_number;
		fs::resize_file(file_path_, file_size_);
	}

	void init_database_file() {
		{ std::ofstream new_file(file_path_, std::ios_base::binary); }
		regions_.mapped_file = std::move(bip::file_mapping(u8_file_path_.c_str(), bip::read_write));
		
		resize_file(1);
		
		meta_ = mapped_ptr_fixed<DatabaseMeta>{0, *this};
		std::memcpy(meta_->magic_thing, cv_magic_thing, sizeof(cv_magic_thing));
		meta_->freed_block = {invalid_block_size, invalid_block_size};
		meta_->free_space = sizeof(DatabaseMeta);
		
		init_tree<MainTree>(32, &meta_->main_tree);
	}

//	std::optional<BlockInfo> try_get_free_block(block_size_t block_size) {
		/*
		for (size_t i = 0; i < free_block_size_list.size(); ++i) {
			auto const size = free_block_size_list[i];
			if (block_size != size) continue;
			
			auto& block = meta_->free_blocks_fixed[i];
			if (block.size == invalid_block_size) return std::nullopt;
				
			BlockInfo bi = block;
			if (block.next_block != invalid_offset) {
				auto next_block = (BlockInfo*)regions_.map_region(block.next_block);
				block = *next_block;
			} else {
				block = {invalid_block_size, invalid_offset};
			}
			return bi;
		}
		*/

//	return std::nullopt;
//	}

	template<typename TTree>
	void init_tree(uint32_t chunks_max, TreeMeta* tree_meta) {
		constexpr auto chunk_size = calc_chunk_size_cla(sizeof(typename TTree::btree_node));

		auto chunk_offset = segregate(chunk_size, chunks_max);
		auto chunk = (Chunk*)regions_.get_ptr(chunk_offset);

		tree_meta->ai.next_free_chunk = chunk->next.offset;
		tree_meta->ai.payload_size = sizeof(typename TTree::btree_node);
		tree_meta->root = mapped_ptr_base{chunk_offset};
		
		mapped_ptr<typename TTree::btree_node> root{tree_meta->root, *this};
		new (root.get()) typename TTree::btree_node();
	}

	// makes linked list of memory blocks started from meta_->free_space
	// returns [offset of the first allocated block]
	offset_t segregate(size_t block_size, uint32_t block_count) {
		assert(block_count > 1);
		assert(block_size < max_window_size);

		const auto bytes_to_allocate = block_size * block_count;
		if (bytes_to_allocate + meta_->free_space > file_size_) {
			resize_file((file_size_ - meta_->free_space > block_size) + 1 + ((bytes_to_allocate + meta_->free_space - file_size_) / max_window_size));
		} 

		BlockIterator it(regions_, meta_->free_space, block_count, block_size);
		const auto first = it.offset();

		for (; !it.is_done(); ++it) {
			reinterpret_cast<Chunk*>(*it)->next.offset = it.next_offset();
		}

		meta_->free_space = it.offset();

		return first;
	}
public:
	MainTree main_tree() { 
		return MainTree{ mapped_ptr_fixed<TreeMeta>{offsetof(DatabaseMeta, main_tree), *this} };
	}
	
	void open() {
		if (!fs::exists(file_path_)) {
			init_database_file();
		} else {
			file_size_ = fs::file_size(file_path_);
			regions_.mapped_file = std::move(bip::file_mapping(u8_file_path_.c_str(), bip::read_write));
			meta_ = mapped_ptr_fixed<DatabaseMeta>{0, *this};
		}
	}

	std::tuple<mapped_ptr_base, void*> alloc_tree_node(mapped_ptr_fixed<TreeMeta>& meta) {	
		auto& ai = meta->ai;
		auto chunk_offset = ai.next_free_chunk;

		if (chunk_offset == invalid_offset) {
			chunk_offset = segregate(calc_chunk_size_cla(ai.payload_size), 32);
		} 

		auto chunk = (Chunk*)regions_.get_ptr(chunk_offset);
		meta->ai.next_free_chunk = chunk->next.offset;

		return std::make_tuple(mapped_ptr_base{chunk_offset}, chunk + 1);
	}

	void free_node(mapped_ptr_base ptr, TreeMeta& meta) {
		*(Chunk*)ptr.map(*this) = *(Chunk*)regions_.get_ptr(meta.ai.next_free_chunk);
		meta.ai.next_free_chunk = ptr.offset;
	}

	void* get_ptr(offset_t offset) {
		return (char*)regions_.get_ptr(offset);
	}

	template<typename Tree>
	bool create_tree(uint64_t tree_id) {
		auto the_tree = main_tree();
		if (the_tree.find(tree_id)) return false;

		MainTree::KeyValue kv;
		kv.key = tree_id;
		
		init_tree<Tree>(32, &kv.value);
		
		the_tree.insert(kv);
		
		return true;
	}

	template<typename Tree> 
	std::optional<Tree> select_tree(uint64_t tree_id) {
		auto the_tree = main_tree();
		if (auto founded = the_tree.find(tree_id); founded) {
			return founded.value();
			//return MainTree{ mapped_ptr_fixed<TreeMeta>{offsetof(DatabaseMeta, main_tree), *this} };
		}

		return std::nullopt;

	}

	Database(fs::path file_path)
		: file_path_{file_path}
		, u8_file_path_{reinterpret_cast<const char*>(file_path.u8string().c_str())}
		, meta_{*this}
	{
		open();
	}
};

} // namespace npdb
