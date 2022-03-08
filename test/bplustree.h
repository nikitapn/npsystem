#pragma once
/*
#include <memory>
#include <array>
#include <optional>

#include "dbtypes.h"
#include "mapped.h"

namespace npdb {

template<typename Key, typename Value>
class mapped_bpt_node_abstract_T {
public:
	using bptree_node_abstract = mapped_bpt_node_abstract_T<Key, Value>;

	virtual bool find(const Key& key) const noexcept = 0;
	virtual int insert(bptree_node_abstract parent, size_t index_in_parent, const Key& key, const Value& value) = 0;
	
	virtual ~mapped_bpt_node_abstract_T() = default;
};

template<template <typename, typename, size_t> typename Node, typename Key, typename Value, size_t Degree>
class mapped_bpt_T
	: public mapped_bpt_node_abstract_T<Key, Value>
	, public mapped_ptr<Node<Key, Value, Degree>> {
public:
	using bptree_node_abstract = mapped_bpt_node_abstract_T<Key, Value>;
	
	virtual bool find(const Key& key) const noexcept {
		return get()->find(key);
	}

	virtual int insert(bptree_node_abstract parent, size_t index_in_parent, const Key& key, const Value& value) final {
		return get()->insert_impl(*this, parent, index_in_parent, key, value);
	}

	mapped_bpt_T(const mapped_ptr_base& other, Database& db)
		: mapped_ptr<Node<Key, Value, Degree>>(node, db)
	{
	}
};

template<typename Key, typename Value, size_t Degree>
class BPTreeNode {
public:
	using Self = BPTreeNode<Key, Value, Degree>;
	static constexpr auto max_keys_size = 2 * Degree - 1;
	static constexpr auto max_childs_size = 2 * Degree;
private:
	std::array<Key, max_keys_size> keys_;
	alignas(16) std::array<mapped_ptr_base, max_childs_size> nodes_;
	size_t n_keys_;
public:
	int insert_impl(
		mapped_bpt_T<BPTreeNode, Key, Value, Degree> self, 
		mapped_bpt_T<BPTreeNode, Key, Value, Degree> parent, 
		size_t index_in_parent, const Key& key) {
		return 0;
	}
};

template<typename Key, typename Value, size_t Degree>
class BPTreeLeaf {
public:
	using Self = BPTreeLeaf<Key, Value, Degree>;
	using Parent = BPTreeNode<Key, Value, Degree>;
	static constexpr size_t max_keys_size = 2 * Degree - 1;
private:
	std::array<Key, max_keys_size> keys_;
	alignas(16) std::array<Value, max_keys_size> values_;
public:
	int insert_impl(
		mapped_bpt_T<BPTreeLeaf, Key, Value, Degree> self, 
		mapped_bpt_T<BPTreeNode, Key, Value, Degree> parent, 
		size_t index_in_parent, const Key& key) {
		return 0;
	}
};

template<typename Key, typename Value, size_t Degree>
class BPTree {
public:
	using bptree_node_abstract = mapped_bpt_node_abstract_T<Key, Value>;
	using bptree_node = mapped_bpt_T<BPTreeNode, Key, Value, Degree>;
	using bptree_leaf = mapped_bpt_T<BPTreeLeaf, Key, Value, Degree>;
private:
	Database& db_;
	size_t max_level_;
	std::unique_ptr<bptree_node_abstract> root_;
public:
	//operator btree_node_ptr() { return root_.get(); }

	bool insert(Key& key) noexcept {
		if (root_->find(key, db_)) return false;
		auto root = root_->insert(key, root_);
		
		if (root_ != root) {
			root_ = root;
			db_.set_root(root_);
		}

		return true;
	}

	bool find(const Key& key) const noexcept {
		return root_->find(key, db_);
	}

	std::unique_ptr<bptree_node_abstract> fetch_node(mapped_ptr_base node, size_t level) {
		if (level == max_level) [[unlikely]] {
			return std::make_unique<bptree_leaf>(node, db_);
		} else {
			return std::make_unique<bptree_node>(node, db_);
		}
	}

	BPTree(Database& db)
		: db_(db)
		, root_(db)
	{
		auto root = db.root();
		if (root.is_invalid()) {
			root_ = make_mapped<btree_node>(db_);
			assert(root_.idx_region == 0 && root_.offset == 0);
			db_.set_root(root_);
		} else {
			root_ = mapped_ptr<btree_node>(root, db_);
		}
	}
};
} // namespace npdb

*/