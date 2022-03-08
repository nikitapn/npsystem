// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dbtypes.h"
#include "mapped.h"
#include "bfsiterator.h"
#include <type_traits>
#include <optional>
#include <ostream>
#include <array>

namespace npdb {
#pragma warning(disable : 26495)

template<typename Key, typename Value>
struct KeyValueT {
	Key key;
	Value value;

	bool operator<(const KeyValueT& kv) const noexcept {
		return this->key < kv.key;
	}

	bool operator<(const Key& key) const noexcept {
		return this->key < key;
	}

	bool operator==(const KeyValueT& kv) const noexcept {
		return this->key == kv.key;
	}

	bool operator==(const Key& key) const noexcept {
		return this->key == key;
	}
};

template<typename Key>
struct KeyValueT<Key, void> {
	Key key;

	bool operator<(const KeyValueT& kv) const noexcept {
		return this->key < kv.key;
	}

	bool operator<(const Key& key) const noexcept {
		return this->key < key;
	}

	bool operator==(const KeyValueT& kv) const noexcept {
		return this->key == kv.key;
	}

	bool operator==(const Key& key) const noexcept {
		return this->key == key;
	}
};

template<typename Key, typename Value>
std::ostream& operator << (std::ostream& os, const KeyValueT<Key, Value>& kv) {
	os << '{' << kv.key << ',' << kv.value << '}';
	return os;
}

template<typename Key>
std::ostream& operator << (std::ostream& os, const KeyValueT<Key, void>& kv) {
	os << kv.key;
	return os;
}

template<typename Key, typename Value, size_t Degree>
class BTreeNode {
	static_assert(Degree >= 2, "minimum degree of b-tree must be >= 2");
public:
	using Self = BTreeNode<Key, Value, Degree>;
	using KeyValue = KeyValueT<Key, Value>;
	using bfs_iterator = _bfs_iterator<Self>;
private:
	template<typename __Key, typename _Value, size_t __Degree>
	friend std::ostream& operator << (std::ostream&, const BTreeNode<__Key, typename _Value, __Degree>&);

	friend bfs_iterator;

	template<typename BNode>
	friend bool verify(mapped_ptr<BNode>, std::optional<typename BNode::KeyValue>, std::optional<typename BNode::KeyValue>);


	static constexpr auto max_keys_size = 2 * Degree - 1;
	static constexpr auto max_childs_size = 2 * Degree;

	std::array<KeyValue, max_keys_size> keys_;
	alignas(16) std::array<mapped_ptr_base, max_childs_size> nodes_;

	size_t n_keys_;
	bool leaf_;

	static size_t find_index(Self* node, const KeyValue& kv) {
		auto count = node->n_keys_;
		/*
		if (count < 32) {
			size_t index = count;
			for (size_t i = 0; i < count; ++i) {
				if (kv < node->keys_[i]) {
					index = i;
					break;
				}
			}
			return index;
		} else {*/
		auto founded = std::lower_bound(node->keys_.begin(), node->keys_.begin() + count, kv);
		return std::distance(node->keys_.begin(), founded);
		//}
		//if (index != std::distance(node->keys_.begin(), founded)) assert(false);
		//return std::distance(node->keys_.begin(), founded);
	}

	static void insert_simple(Self* node, mapped_ptr<Self>& l, mapped_ptr<Self>& r, size_t index, const KeyValue& kv) {
		assert(!node->is_full());
		std::move(
			node->nodes_.begin() + index + 1,
			node->nodes_.begin() + node->nodes_size(),
			node->nodes_.begin() + index + 2);

		node->nodes_[index] = l;
		node->nodes_[index + 1] = r;

		std::move(
			node->keys_.begin() + index,
			node->keys_.begin() + node->keys_size(),
			node->keys_.begin() + index + 1);

		node->keys_[index] = kv;

		node->n_keys_ = node->n_keys_ + 1;
	}

	static std::tuple<mapped_ptr<Self>, mapped_ptr<Self>, KeyValue>
		split_half(mapped_ptr_fixed<TreeMeta>& meta, mapped_ptr<Self>& node) {
		assert(node->is_full() && !node->is_leaf());

		constexpr auto ksize = (max_keys_size - 1) / 2;
		constexpr auto nsize = max_childs_size / 2;

		node->n_keys_ = ksize;

		auto new_right_node = make_mapped<Self>(node.db(), meta, ksize, false);
		std::copy(node->keys_.begin() + ksize + 1, node->keys_.end(), new_right_node->keys_.begin());
		std::copy(node->nodes_.begin() + nsize, node->nodes_.end(), new_right_node->nodes_.begin());

		return { node, new_right_node, node->keys_[ksize] };
	}
public:
	bool is_leaf() const noexcept {
		return leaf_;
	}

	bool is_full() const noexcept {
		return n_keys_ == max_keys_size;
	}

	size_t keys_size() const noexcept {
		return n_keys_;
	}

	size_t nodes_size() const noexcept {
		assert(is_leaf() == false);
		return n_keys_ + 1;
	}


	[[nodiscard]] mapped_ptr<Self> insert(mapped_ptr_fixed<TreeMeta>& meta, const KeyValue& kv, mapped_ptr<Self>& root) {
		mapped_ptr<Self> new_root(root);
		insert_impl(meta, kv, root, root, -1, new_root);
		return new_root;
	}

	std::optional<
		std::tuple<
		mapped_ptr<Self>, 
		mapped_ptr<Self>, KeyValue>
	>
		static insert_impl(
			mapped_ptr_fixed<TreeMeta>& meta, 
			const KeyValue& kv, 
			mapped_ptr<Self>& self, 
			mapped_ptr<Self>& parent, 
			size_t index_in_parent, 
			mapped_ptr<Self>& new_root) 
	{
		if (self->is_leaf()) {
			if (self->n_keys_ == 0) {
				self->keys_[0] = kv;
				self->n_keys_ = self->n_keys_ + 1;
			} else if (self->is_full()) {
				// split current leaf node into 2
				constexpr auto left_childs_keys_size = max_keys_size / 2;
				constexpr auto right_childs_keys_size = max_keys_size - left_childs_keys_size - 1;

				auto median_kv = self->keys_[left_childs_keys_size];

				auto new_right_node = make_mapped<Self>(parent.db(), meta, right_childs_keys_size, true);
				std::copy(self->keys_.begin() + left_childs_keys_size + 1, self->keys_.end(), new_right_node->keys_.begin());

				self->n_keys_ = left_childs_keys_size;

				if (kv < median_kv) {
					insert_impl(meta, kv, self, parent, -1, new_root);
				} else {
					new_right_node->insert_impl(meta, kv, new_right_node, parent, -1, new_root);
				}

				if (parent == self) [[unlikely]] {
					new_root = make_mapped<Self>(parent.db(), meta, 1, false);
					new_root->keys_[0] = median_kv;
					new_root->nodes_[0] = self;
					new_root->nodes_[1] = new_right_node;
				} else if (!parent->is_full()) {
					auto index = find_index(parent.get(), median_kv);
					assert(index == index_in_parent);
					insert_simple(parent.get(), self, new_right_node, index_in_parent, median_kv);
				} else {
					// split parent
					auto x = split_half(meta, parent);
					auto node_to = median_kv < std::get<2>(x) ? std::get<0>(x) : std::get<1>(x);
					size_t index_to = find_index(node_to.get(), median_kv);

					insert_simple(node_to.get(), self, new_right_node, index_to, median_kv);

					return x;
				}
			} else {
				// there is a space in the leaf
				auto index = find_index(self.get(), kv);
				if (index != self->n_keys_) {
					std::move(&self->keys_[index], &self->keys_[self->n_keys_], std::next(&self->keys_[index]));
				}
				self->keys_[index] = kv;
				self->n_keys_ = self->n_keys_ + 1;
			}
		} else { // not a leaf
			size_t index = find_index(self.get(), kv);
			auto node = mapped_ptr<Self>(self->nodes_[index], self.db());
			auto x = node->insert_impl(meta, kv, node, self, index, new_root);
			if (x) {
				auto [left, right, kv] = *x;
				if (parent == self) {
					new_root = make_mapped<Self>(parent.db(), meta, 1, false);
					new_root->keys_[0] = kv;
					new_root->nodes_[0] = left;
					new_root->nodes_[1] = right;
				} else {
					if (parent->is_full()) {
						auto y = split_half(meta, parent);

						auto node_to = kv < std::get<2>(y) ? std::get<0>(y) : std::get<1>(y);
						size_t index_to = find_index(node_to.get(), kv);
						insert_simple(node_to.get(), left, right, index_to, kv);

						return y;
					} else {
						auto index = find_index(parent.get(), kv);
						assert(index == index_in_parent);
						insert_simple(parent.get(), left, right, index_in_parent, kv);
					}

				}
			}
		}
		return {};
	}

	static auto find(mapped_ptr<Self> from, const Key& key, Database& db) noexcept {
		using return_t = std::conditional_t<std::is_same_v<Value, void>, bool, std::optional<Value>>;
		
		auto ptr_unsafe = from.get();

		auto it = std::lower_bound(ptr_unsafe->keys_.begin(), ptr_unsafe->keys_.begin() + ptr_unsafe->n_keys_, key);
		auto index = std::distance(ptr_unsafe->keys_.begin(), it);

		if (index != ptr_unsafe->n_keys_ && ptr_unsafe->keys_[index] == key) {
			if constexpr (std::is_same_v<Value, void>) {
				return true;
			} else {
				return std::optional<Value>(ptr_unsafe->keys_[index].value);
			}
		}

		if (ptr_unsafe->is_leaf()) return return_t{};

		return find(mapped_ptr<Self>{ptr_unsafe->nodes_[index], db}, key, db);
	}

	BTreeNode()
		: n_keys_(0)
		, leaf_(true)
	{
	}

	BTreeNode(size_t n_KeyValues, bool leaf)
		: n_keys_(n_KeyValues)
		, leaf_(leaf)
	{
	}
};

template<typename __Key, typename __Value, size_t __Degree>
std::ostream& operator << (std::ostream& os, const BTreeNode<__Key, __Value, __Degree>& node) {
	if (node.n_keys_ == 0) {
		os << "[]";
		return os;
	}
	
	os << '[';
	for (size_t i = 0; i < node.n_keys_ - 1; ++i) {
		os << node.keys_[i] << ' ';
	}
	os << node.keys_[node.n_keys_ - 1] << ']';
	return os;
}

template<typename BNode>
bool verify(
	mapped_ptr<BNode> node, 
	std::optional<typename BNode::KeyValue> left_kv, 
	std::optional<typename BNode::KeyValue> right_kv) {
	for (size_t i = 0; i < node->n_keys_ - 1; ++i) {
		if (node->keys_[i + 1] < node->keys_[i]) return false;
	}

	if (left_kv) {
		for (size_t i = 0; i < node->n_keys_; ++i) {
			if (node->keys_[i] < left_kv) return false;
		}
	}

	if (right_kv) {
		for (size_t i = 0; i < node->n_keys_; ++i) {
			if (right_kv < node->keys_[i]) return false;
		}
	}

	if (node->is_leaf() == false) {
		for (size_t i = 0; i < node->n_keys_; ++i) {
			bool ok = verify<BNode>(
				mapped_ptr<BNode>(node->nodes_[i], node.db()),
				i == 0 ? std::optional<typename BNode::KeyValue>{} : node->keys_[i - 1], 
				node->keys_[i]
				);
			if (!ok) return ok;
		}
		return verify<BNode>(
			mapped_ptr<BNode>(node->nodes_[node->n_keys_], node.db()),
			node->keys_[node->n_keys_ - 1], 
			std::optional<typename BNode::KeyValue>{}
			);
	}

	return true;
}

template<typename Key, typename Value, size_t Degree>
class BTree {
public:
	using btree_node = BTreeNode<Key, Value, Degree>;
	using btree_node_ptr = btree_node*;
	using KeyValue = typename btree_node::KeyValue;
private:
	mapped_ptr_fixed<TreeMeta> meta_;
	mapped_ptr<btree_node> root_;
	Database& db_;
public:
	auto root() {return root_; }

	bool insert(const KeyValue& kv) noexcept {
		if (find(kv.key)) return false;
		auto root = root_->insert(meta_, kv, root_);
		if (root_ != root) {
			root_ = root;
			meta_->root = root;
		}
		return true;
	}

	auto find(const Key& key) const noexcept 
		-> std::conditional_t<std::is_same_v<Value, void>, bool, std::optional<Value>> {
		return btree_node::find(root_, key, db_);
	}

	BTree(mapped_ptr_fixed<TreeMeta> meta)
		: meta_{meta}
		, root_{meta->root, meta.db()}
		, db_{meta.db()}
	{
	}
};

} // namespace npdb