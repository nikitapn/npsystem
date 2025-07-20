#pragma once

/** 
 * A Trie implementation for storing and matching patterns
 * This implementation supports characters 'a' to 'z', '0' to '9', '.', '-', and '_'.
 */

#include <vector>
#include <array>
#include <map>
#include <cstdint>
#include <string_view>

namespace nplib::trie {

namespace detail {

class TrieNode {
  friend class TriePool;

  std::uint16_t                 index_;
  std::array<std::uint16_t, 39> children;
  bool                          is_end_of_word = false;

  TrieNode(std::uint16_t index, bool is_end = false)
    : index_(index)
    , is_end_of_word(is_end)
  {
    children.fill(static_cast<std::uint16_t>(-1));
  }
public:
  static constexpr std::uint16_t from_char_to_index(char c) {
    if (c >= 'a' && c <= 'z')
      return static_cast<std::uint16_t>(c - 'a');
    if (c >= '0' && c <= '9')
      return static_cast<std::uint16_t>(c - '0' + 26);
    if (c == '.') return 36;
    if (c == '-') return 37;
    if (c == '_') return 38;
    return static_cast<std::uint16_t>(-1);
  }

  std::uint16_t index() const {
    return index_;
  }

  bool is_end() const {
    return is_end_of_word;
  }

  void set_end(bool end) {
    is_end_of_word = end;
  }

  const TrieNode* get_child(char c) const;

  TrieNode* get_child(char c) {
    return const_cast<TrieNode*>(static_cast<const TrieNode*>(this)->get_child(c));
  }

  void set_child(char c, std::uint16_t poolIndex) {
    auto index = from_char_to_index(c);
    if (index < children.size()) {
      children[index] = poolIndex;
    }
  }
};

class TriePtr {
  std::uint16_t index_;
public:
  std::uint16_t index() const { return index_; }
  TrieNode& operator*();
  TrieNode* operator->();
  TrieNode* get();
  const TrieNode* get() const;

  explicit TriePtr(std::uint16_t index) : index_(index) {}
};

class TriePool {
  std::vector<TrieNode> pool_;
public:

  static TriePool& instance() {
    static TriePool instance;
    return instance;
  }

  TriePtr allocateNode() {
    auto index = static_cast<std::uint16_t>(pool_.size());
    pool_.emplace_back(TrieNode(index));
    return TriePtr(index);
  }

  TrieNode* getNode(std::uint16_t index) {
    return &pool_[index];
  }

  const TrieNode* getNode(std::uint16_t index) const {
    return &pool_[index];
  }

  TriePool() {
    pool_.reserve(2048); // Reserve space for 2048 nodes initially
  }
};

inline TrieNode& TriePtr::operator*() {
  return *TriePool::instance().getNode(index_);
}

inline TrieNode* TriePtr::operator->() {
  return TriePool::instance().getNode(index_);
}

inline TrieNode* TriePtr::get() {
  return TriePool::instance().getNode(index_);
}

inline const TrieNode* TriePtr::get() const {
  return TriePool::instance().getNode(index_);
}

inline const TrieNode* TrieNode::get_child(char c) const {
  auto index = from_char_to_index(c);
  if (index < children.size() && children[index] != static_cast<std::uint16_t>(-1)) {
    return TriePool::instance().getNode(children[index]);
  }
  return nullptr;
}

class Trie {
  TriePtr root_;
public:
  bool insert(const std::string_view pattern) {
    TriePtr current = root_;
    for (char c : pattern) {
      if (TrieNode::from_char_to_index(c) == static_cast<std::uint16_t>(-1)) {
        // Invalid character for Trie
        return false;
      }

      TrieNode* child = current->get_child(c);
      if (child) {
        current = TriePtr(child->index());
      } else {
        // Allocate a new node for this character
        TriePtr newChild = TriePool::instance().allocateNode();
        current->set_child(c, newChild.index());
        current = newChild;
      }
    }
    current->set_end(true); // Mark the end of the word
    return true; // Successfully inserted
  }

  bool search(const std::string_view pattern) const {
    const TrieNode* current = root_.get();
    for (char c : pattern) {
      current = current->get_child(c);
      if (!current) {
        return false; // Not found
      }
    }
    return current->is_end();
  }

  Trie() : root_(TriePool::instance().allocateNode()) {}
};
} // namespace detail

class TrieSet {
  // Trie to hold exact matches from the beginning of the string
  detail::Trie root_;
  // A set of tries, each trie can represent a different pattern set
  // The key is the size of the pattern, and the value is a Trie instance
  std::map<size_t, detail::Trie> wildcard_tries_;

  bool insert_exact(const std::string_view pattern) {
    return root_.insert(pattern);
  }

  bool insert_wildcard(const std::string_view pattern) {
    size_t size = pattern.size();
    auto it = wildcard_tries_.find(size);
    if (it == wildcard_tries_.end()) {
      // Create a new trie for this size
      detail::Trie newTrie;
      if (!newTrie.insert(pattern)) {
        return false; // Insertion failed
      }
      wildcard_tries_.emplace(size, std::move(newTrie));
    } else {
      // Insert into existing trie
      if (!it->second.insert(pattern)) {
        return false; // Insertion failed
      }
    }
    return true; // Successfully inserted
  }

public:
  bool insert(const std::string_view pattern) {
    if (pattern.empty()) {
      // Cannot insert empty pattern
      return false; 
    }

    if (pattern.find_first_not_of("*abcdefghijklmnopqrstuvwxyz0123456789._-") != std::string_view::npos) {
      // Invalid character in pattern
      return false;
    }

    if (pattern[0] == '*') {
      // If the pattern starts with '*', treat it as a wildcard
      return insert_wildcard(pattern.substr(1)); // Skip the '*' character
    } else {
      // Otherwise, treat it as an exact match
      return insert_exact(pattern);
    }
  }


  bool search(const std::string_view str) const {
    // First check the exact match in the root trie
    if (root_.search(str))
      return true;

    // Then check the wildcard tries
    // Assuming wildcard tries are for patterns that can match the end of the string
    const auto str_size = str.size();
    for (const auto& [size, trie] : wildcard_tries_) {
      if (size > str_size)
        continue; // Skip tries that are larger than the string

      if (trie.search(std::string_view(str.data() + str_size - size, size)))
        return true; // Found a match in the wildcard trie
    }
    return false; // No match found
  }
};

} // namespace nplib::trie