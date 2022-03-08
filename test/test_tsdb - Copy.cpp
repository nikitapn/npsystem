#include <iostream>
#include <utility>
#include <array>
#include <assert.h>
#include <queue>
#include <nplib/utils/helper.h>
#include <concepts>
#include <optional>
#include <random>
#include <fstream>
#include <filesystem>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <unordered_set>

template<typename T>
class _bfs_iterator {
	using Key = typename T::Key;
	using Item = std::pair<T*, size_t>;

	T& root_;
	Item current_;
	std::queue<Item> queue_;
public:
	bool next() {
		if (queue_.empty()) return false;

		current_ = queue_.front();
		queue_.pop();

		if (current_.first->is_leaf() == false) {
			auto next_level = current_.second + 1;
			for (size_t i = 0; i <= current_.first->n_keys_; ++i) {
				if (current_.first->nodes_[i]) {
					queue_.push({ current_.first->nodes_[i], next_level });
				}
			}
		}
		return true;
	}

	auto value() noexcept { return current_; }

	_bfs_iterator(T& root)
		: root_(root)
		, current_({ &root, 0 })
	{
		if (root_.is_leaf() == false) {
			for (size_t i = 0; i <= current_.first->n_keys_; ++i) {
				if (current_.first->nodes_[i]) {
					queue_.push({ current_.first->nodes_[i], 1 });
				}
			}
		}
	}
};

namespace bip = boost::interprocess;
namespace fs = std::filesystem;

using region_t = uint16_t;
using offset_t = uint16_t;

constexpr auto invalid_region = static_cast<region_t>(-1);
constexpr auto invalid_offset = static_cast<offset_t>(-1);

class mapped_ptr_base {
public:
	uint16_t idx_region;
	uint16_t offset;

	mapped_ptr_base(uint16_t _idx_region, uint16_t _offset)
		: idx_region(_idx_region)
		, offset(_offset)
	{
	}

	template<typename U>
	mapped_ptr_base* get_chunk(U& db, size_t chunk_size) {
		return reinterpret_cast<mapped_ptr_base*>(
			static_cast<char*>(db.get_region_base_ptr(idx_region)) + offset * chunk_size
			);
	}
};

template<typename T>
class mapped_ptr : public mapped_ptr_base {
public:
	template<typename U>
	mapped_ptr* get_chunk(U& db) {
		return static_cast<mapped_ptr*>(mapped_ptr_base::get_chunk(db, sizeof(T) + sizeof(mapped_ptr_base)));
	}

	template<typename U>
	T* get(U& db) {
		return reinterpret_cast<T*>(get_chunk(db) + 1);
	}

	mapped_ptr(uint16_t _idx_region, uint16_t _offset)
		: mapped_ptr_base(_idx_region, _offset)
	{
	}
	mapped_ptr(const mapped_ptr&) = default;
	mapped_ptr(const mapped_ptr_base& other) :
		mapped_ptr_base(other) {}
};



struct b_tree_node_base {};
struct b_tree_leaf_base {};

constexpr size_t region_max = 65536;
constexpr size_t offset_max = 65536;

//constexpr size_t region_map_size = (1 << 20); // 1 MB
//constexpr size_t region_size = (1 << 12); // 1 page
constexpr size_t region_size = 256;
constexpr size_t max_file_gb = region_max * region_size / (1 << 30);

static constexpr size_t meta_size = 4096;

struct mapped_region {
	region_t index;
	bip::mapped_region region;

	mapped_region(region_t idx, const bip::file_mapping& m_file)
		: index(idx)
		, region(bip::mapped_region(m_file, bip::read_write, meta_size + idx * region_size, region_size))
	{
	}

	bool operator < (const mapped_region& val) const noexcept { return index < val.index; }
	bool operator == (const mapped_region& val) const noexcept { return index == val.index; }
	bool operator == (const region_t val) const noexcept { return index == val; }
};

namespace std {
	template<>
	struct hash<mapped_region> {
		size_t operator()(const mapped_region& val) const noexcept{
			return std::hash<region_t>{}(val.index);
		}
	};
}
class Database;

void test_dbdb(Database& db);

class Database {
	friend void test_dbdb(Database& db);
	static constexpr size_t page_size = 4096;
	static constexpr size_t cached_regions_index_begin = 1;
	//static constexpr size_t node_size = sizeof(BTreeNode<Key, 100>)*2;

	fs::path file_path_;
	std::string u8_file_path_;

	bip::file_mapping m_file_;
	bip::mapped_region mr_header_;
	
	std::unordered_set<mapped_region> regions_;

	size_t node_size_;

	struct DatabaseHeader {
		uint64_t nodes_count = 0;
		region_t regions_count = 0;
		mapped_ptr_base next_free_chunk = { invalid_region , invalid_offset };
	} *header_ = nullptr;

	void init_database_file() {
		{
			std::ofstream new_file(file_path_, std::ios_base::binary);
			std::fill_n(std::ostreambuf_iterator<char>(new_file), meta_size, '\0');
		}
		m_file_ = std::move(bip::file_mapping(u8_file_path_.c_str(), bip::read_write));
		mr_header_ = std::move(bip::mapped_region(m_file_, bip::read_write, 0, meta_size));
		header_ = new (mr_header_.get_address()) DatabaseHeader;
	}
	void* load_region(region_t region) {
		auto [it, ok] = regions_.emplace(region, m_file_);
		return it->region.get_address();
	}
public:
	void open() {
		if (!fs::exists(file_path_)) {
			init_database_file();
		} else {
			m_file_ = std::move(bip::file_mapping(u8_file_path_.c_str(), bip::read_write));
			mr_header_ = std::move(bip::mapped_region(m_file_, bip::read_write, 0, meta_size));
			header_ = static_cast<DatabaseHeader*>(mr_header_.get_address());
		}
	}

	std::tuple<mapped_ptr_base, void*> alloc_node() {
		const size_t chunk_size = node_size_ + sizeof(mapped_ptr_base);
		assert(2 * chunk_size < region_size);
		
		mapped_ptr_base ptr = {invalid_region, invalid_offset};
		void* raw;

		if (header_->regions_count == 0) {
			fs::resize_file(file_path_, meta_size + region_size);
			segregate_region(chunk_size, 0, nullptr);
			ptr = { 0, 0 };
			raw = ptr.get_chunk(*this, chunk_size) + 1;
			header_->next_free_chunk = { 0, 1 };
			header_->regions_count = 1;
			
		} else {
			ptr = header_->next_free_chunk;
			auto next = ptr.get_chunk(*this, chunk_size);
			raw = next + 1;
			if (next->idx_region == invalid_region) {
				fs::resize_file(file_path_, meta_size + ((size_t)header_->regions_count + 1) * region_size);
				segregate_region(chunk_size, header_->regions_count, next);
				header_->regions_count = header_->regions_count + 1;
			}
			header_->next_free_chunk = *next;
		}

		return std::make_tuple(ptr, raw);
	}

	void free_node(mapped_ptr_base ptr) {
		const size_t chunk_size = node_size_ + sizeof(mapped_ptr_base);
		assert(2 * chunk_size < region_size);
		*ptr.get_chunk(*this, chunk_size) = *header_->next_free_chunk.get_chunk(*this, chunk_size);
		header_->next_free_chunk = ptr;
	}

	void segregate_region(size_t chunk_size, region_t idx_region, mapped_ptr_base* last) {
		char* region_ptr = (char*)get_region_base_ptr(idx_region);

		if (last) {
			assert(last->idx_region == invalid_region && last->offset == invalid_offset);
			last->idx_region = idx_region;
			last->offset = 0;
		}

		char* last_chunk = region_ptr + (region_size - region_size % chunk_size - chunk_size);
		offset_t offset = 1;
		for (char* ptr = region_ptr; ptr < last_chunk; ptr += chunk_size) {
			*reinterpret_cast<mapped_ptr_base*>(ptr) = { idx_region, offset++ };
		}
		*reinterpret_cast<mapped_ptr_base*>(last_chunk) = { invalid_region, invalid_offset };
	}

	void* get_region_base_ptr(region_t idx) {
		return load_region(idx);
	}

	Database(fs::path file_path, size_t node_size) 
		: file_path_(file_path)
		, u8_file_path_(reinterpret_cast<const char*>(file_path.u8string().c_str()))
		, node_size_(node_size)
	{
		open();
	}
};

template<typename T, typename... Args>
std::tuple<mapped_ptr<T>, T*> make_mapped(Database& db, Args... args) {
	auto [ptr, raw] = db.alloc_node();
	new (raw) T(std::forward<Args...>(args)...);
	return std::make_tuple<mapped_ptr<T>, T*>(ptr, static_cast<T*>(raw));
}

void test_dbdb(Database& db) {
	/*
	for (int i = 0; i < 46; ++i) {
		auto ptr = db.alloc_node();
		int ii = 0;
	}*/
	
	//db.free_node({ 2, 0 });
}


template<typename _Key, size_t Degree>
class BTreeNode : public b_tree_node_base {
	static_assert(Degree >= 2, "minimum degree of b-tree must be >= 2");
public:
	using Key = _Key;
	using Self = BTreeNode<Key, Degree>;
	using bfs_iterator = _bfs_iterator<Self>;
private:
	template<typename __Key, size_t __Degree>
	friend std::ostream& operator << (std::ostream&, const BTreeNode<__Key, __Degree>&);
	
	friend bfs_iterator;

	template<typename Key, size_t Degree>
	friend bool verify(BTreeNode<Key, Degree>*,
		typename BTreeNode<Key, Degree>::Key*,
		typename BTreeNode<Key, Degree>::Key*);

	static constexpr auto max_keys_size = 2 * Degree - 1;
	static constexpr auto max_childs_size = 2 * Degree;

	std::array<Key, max_keys_size> keys_;
	std::array<mapped_ptr<Self>, max_childs_size> nodes_;
	
	size_t n_keys_;
	bool leaf_;

	static size_t find_index(Self* node, const Key& key) {
		auto count = node->n_keys_;
		if (count < 32) {
			size_t index = count;
			for (size_t i = 0; i < count; ++i) {
				if (key < node->keys_[i]) {
					index = i;
					break;
				}
			}
			return index;
		} else {
			auto founded = std::lower_bound(node->keys_.begin(), node->keys_.begin() + count, key);
			return std::distance(node->keys_.begin(), founded);
		}
		//if (index != std::distance(node->keys_.begin(), founded)) assert(false);
		//return std::distance(node->keys_.begin(), founded);
	}

	static void insert_simple(Self* node, Self* l, Self* r, size_t index, Key& key) {
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

		node->keys_[index] = key;

		node->n_keys_ = node->n_keys_ + 1;
	}

	static std::tuple<Self*, Self*, Key> split_half(Self* node, Database& db) {
		assert(node->is_full() && !node->is_leaf());

		constexpr auto ksize = (max_keys_size - 1) / 2;
		constexpr auto nsize = max_childs_size / 2;

		node->n_keys_ = ksize;

		auto new_right_node = make_mapped<BTreeNode>(db, ksize, false);
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

	Self* insert(Key& key, Database& db) {
		auto root = this;
		insert_impl(key, this, -1, root, db);
		return root;
	}

	std::optional<std::tuple<Self*, Self*, Key>> 
		insert_impl(Key& key, Self* parent, size_t index_in_parent, Self*& new_root, Database& db) {
		if (is_leaf()) {
			if (n_keys_ == 0) {
				//keys_ = new Key[max_keys_size];
				keys_[0] = key;
				n_keys_ = n_keys_ + 1;
			} else if (is_full()) {
				// split current leaf node into 2
				constexpr auto left_childs_keys_size = max_keys_size / 2;
				constexpr auto right_childs_keys_size = max_keys_size - left_childs_keys_size - 1;

				auto median_key = keys_[left_childs_keys_size];
				
				auto new_right_node = new BTreeNode(right_childs_keys_size, true);
				std::copy(keys_.begin() + left_childs_keys_size + 1, keys_.end(), new_right_node->keys_.begin());

				n_keys_ = left_childs_keys_size;

				if (key < median_key) {
					insert_impl(key, nullptr, -1, new_root);
				} else {
					new_right_node->insert_impl(key, nullptr, -1, new_root);
				}

				if (parent == this) [[unlikely]] {
					new_root = new BTreeNode(1, false);
					new_root->keys_[0] = median_key;
					new_root->nodes_[0] = this;
					new_root->nodes_[1] = new_right_node;
				} else if (!parent->is_full()) {
					auto index = find_index(parent, median_key);
					assert(index == index_in_parent);
					insert_simple(parent, this, new_right_node, index_in_parent, median_key);
				} else {
					// split parent
					auto x = split_half(parent);
					auto node_to = median_key < std::get<2>(x) ? std::get<0>(x) : std::get<1>(x);
					size_t index_to = find_index(node_to, median_key);

					insert_simple(node_to, this, new_right_node, index_to, median_key);

					return x;
				}
			} else {
				// there is a space in the leaf
				auto index = find_index(this, key);
				if (index != n_keys_) {
					std::move(&keys_[index], &keys_[n_keys_], std::next(&keys_[index]));
				} 
				keys_[index] = key;
				n_keys_ = n_keys_ + 1;
			}
		} else { // not a leaf
			size_t index = find_index(this, key);
			auto node = nodes_[index];
			auto x = node->insert_impl(key, this, index, new_root);
			if (x) {
				auto [left, right, key] = *x;
				if (parent == this) {
					new_root = new BTreeNode(1, false);
					new_root->keys_[0] = key;
					new_root->nodes_[0] = left;
					new_root->nodes_[1] = right;
				} else {
					if (parent->is_full()) {
						auto y = split_half(parent);
						
						auto node_to = key < std::get<2>(y) ? std::get<0>(y) : std::get<1>(y);
						size_t index_to = find_index(node_to, key);
						insert_simple(node_to, left, right, index_to, key);
						
						return y;
					} else {
						auto index = find_index(parent, key);
						assert(index == index_in_parent);
						insert_simple(parent, left, right, index_in_parent, key);
					}
				}
			}
		}
		return {};
	}

	bool find(const Key& key, Database& db) const noexcept {
		size_t index;
		if (n_keys_ < 32) {
			index = n_keys_;
			for (size_t i = 0; i < n_keys_; ++i) {
				if (!(keys_[i] < key)) {
					index = i;
					break;
				}
			}
		} else {
			auto founded = std::lower_bound(keys_.begin(), keys_.begin() + n_keys_, key);
			index = std::distance(keys_.begin(), founded);
		}

		if (index != n_keys_ && keys_[index] == key) return true;
		if (is_leaf()) return false;
		
		return nodes_[index]->find(key, db);
	}

	bfs_iterator get_bfs_it() noexcept {
		return bfs_iterator{ *this };
	}

	BTreeNode()
		: n_keys_(0)
		, leaf_(true) 
	{
	}

	BTreeNode(size_t n_keys, bool leaf) 
		: n_keys_(n_keys)
		, leaf_(leaf) 
	{
	}
};

template<typename _Key, size_t Degree>
std::ostream& operator << (std::ostream& os, const BTreeNode<_Key, Degree>& node) {
	os << '[';
	for (size_t i = 0; i < node.n_keys_ - 1; ++i) {
		os << node.keys_[i] << ' ';
	}
	os << node.keys_[node.n_keys_ - 1] << ']';
	return os;
}

struct Key {
	int value;
	bool operator<(const Key& k) const noexcept {
		return value < k.value;
	}
	bool operator==(const Key& k) const noexcept {
		return value == k.value;
	}
};

std::ostream& operator << (std::ostream& os, const Key& key) {
	os << key.value;
	return os;
}

template<typename Key, size_t Degree>
bool verify(BTreeNode<Key, Degree>* node, 
	typename BTreeNode<Key, Degree>::Key* left_key, 
	typename BTreeNode<Key, Degree>::Key* right_key) {
	for (size_t i = 0; i < node->n_keys_ - 1; ++i) {
		if (node->keys_[i + 1] < node->keys_[i]) return false;
	}

	if (left_key != nullptr) {
			for (size_t i = 0; i < node->n_keys_; ++i) {
			if (node->keys_[i] < *left_key) return false;
		}
	}

	if (right_key != nullptr) {
			for (size_t i = 0; i < node->n_keys_; ++i) {
			if (*right_key < node->keys_[i]) return false;
		}
	}

	if (node->is_leaf() == false) {
		for (size_t i = 0; i < node->n_keys_; ++i) {
			bool ok = verify(
				node->nodes_[i],
				i == 0 ? nullptr : &node->keys_[i - 1],
				&node->keys_[i]);
			if (!ok) return ok;
		}
		return verify(
			node->nodes_[node->n_keys_],
			&node->keys_[node->n_keys_ - 1],
			nullptr);
	} 

	return true;
}

template<typename _Key, size_t Degree>
class BTree {
public:
	using btree_node = BTreeNode<Key, Degree>;
	using btree_node_ptr = btree_node*;
private:
	Database& db_;
	btree_node_ptr root_;
public:
	BTree(Database& db_) 
		: db_(db) {
		//root_ = new btree_node();
	}

	operator btree_node_ptr() { return root_; }

	bool insert(Key& key) noexcept {
		if (root_->find(key, db_)) return false;
		root_ = root_->insert(key, db_);
		return true;
	}

	bool find(const Key& key) const noexcept {
		return root_->find(key, db_);
	}
};

using btree = BTree<Key, 3>;

/*
void print_node(btree::btree_node_ptr node) {
	auto it = node->get_bfs_it();
	size_t level = 0;
	do {
		auto node = it.value();
		if (node.second != level) {
			level = node.second;
			std::cout << '\n';
		}
		std::cout << *node.first << ' ';
	} while (it.next());
	std::cout << '\n';
}

void insert_print(btree& root, int key) {
	static int n = 1;
	std::cout << "->" << n++ <<" insert: " << key << '\n';
	if (root.insert(Key{ key }) == false) {
		std::cout << "Error: key exists...\n";
		assert(false);
	}
	print_node(root);
	std::cout << '\n';
}

void print_tree(btree& root, int key) {
	std::cout << "->insert: " << key << '\n';
	print_node(root);
	std::cout << '\n';
}

std::vector<int> load_data(const int max, const char* filename) {
	std::ifstream file(filename);
	std::vector<int> data;
	data.reserve(max);
	if (file) {
		std::copy(std::istream_iterator<int>(file), std::istream_iterator<int>(), std::back_inserter(data));
	} else {
		data.resize(max);
		for (int i = 0; i < max; ++i) data[i] = i;

		std::random_device rd;
    std::mt19937 g(rd());
		std::shuffle(data.begin(), data.end(), g);

		std::ofstream file(filename);
		std::copy(data.begin(), data.end(), std::ostream_iterator<int>(file, "\n"));
	}
	return data;
}
*/

int test_tsdb() {
	try {
		Database db("d:/temp/my.db", sizeof(btree::btree_node));
		test_dbdb(db);
		return 0;
		/*
		constexpr size_t size = 128;
		auto data = load_data(size, "data.bin");
		btree the_tree;

		for (size_t i = 1; i <= size; ++i) {
			insert_print(the_tree, data[i - 1]);

			for (size_t j = 0; j < i; ++j) {
				if (the_tree.find(Key{ data[j] }) == false) {
					std::cout << "Error: " << data[j] << " was not found...\n";
					assert(false);
				}
			}
		}

		if (verify((btree::btree_node_ptr)the_tree, nullptr, nullptr) == false) {
			std::cout << "Error: verification failed...\n";
			assert(false);
		}
		*/
	} catch (std::exception& e) {
		std::cerr << e.what();
		return -1;
	}

	return 0;
}