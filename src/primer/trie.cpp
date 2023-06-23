#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  /** throw NotImplementedException("Trie::Get is not implemented."); */

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.

  if (!root_) {
    return nullptr;
  }
  std::shared_ptr<const TrieNode> cur = root_;
  for (char c : key) {
    auto children = cur->children_;
    if (children.find(c) == children.end()) {
      return nullptr;
    }
    cur = children[c];
  }
  auto terminal_node = dynamic_cast<const TrieNodeWithValue<T> *>(cur.get());
  return terminal_node != nullptr ? terminal_node->value_.get() : nullptr;
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  /** throw NotImplementedException("Trie::Put is not implemented."); */

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  std::shared_ptr<T> last_val = std::make_shared<T>(std::move(value));
  std::shared_ptr<TrieNode> new_root;
  if (key.empty()) {
    if (root_) {
      new_root = std::make_shared<TrieNodeWithValue<T>>(root_->children_, last_val);
    } else {
      new_root = std::make_shared<TrieNodeWithValue<T>>(last_val);
    }
    return Trie(new_root);
  }

  std::shared_ptr<TrieNode> cur;
  std::shared_ptr<TrieNode> child;
  new_root = root_ ? std::shared_ptr(root_->Clone()) : std::make_shared<TrieNode>();
  for (size_t i = 0; i < key.size(); i++) {
    char c = key[i];
    cur = i == 0 ? new_root : child;
    auto children = cur->children_;
    if (i != key.size() - 1) {
      if (children.find(c) != children.end()) {
        child = std::shared_ptr(children[c]->Clone());
      } else {
        child = std::make_shared<TrieNode>();
      }
    } else {
      if (children.find(c) != children.end()) {
        auto orig_children = children[c]->children_;
        child = std::make_shared<TrieNodeWithValue<T>>(orig_children, last_val);
      } else {
        child = std::make_shared<TrieNodeWithValue<T>>(last_val);
      }
    }

    // modify on const TrieNode
    // auto tmp = cur->Clone();
    // tmp->children_[c] = child;
    // cur = std::shared_ptr(std::move(tmp));
    cur->children_[c] = child;
  }

  return Trie(new_root);
}

auto Trie::Remove(std::string_view key) const -> Trie {
  /** throw NotImplementedException("Trie::Remove is not implemented."); */

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.

  size_t last_val_idx = key.size() - 1;
  std::shared_ptr<const TrieNode> node = root_;
  for (size_t i = 0; i < key.size(); i++) {
    char c = key[i];
    auto children = node->children_;
    if (children.find(c) == children.end() || (i == key.size() - 1 && !children[c]->is_value_node_)) {
      return *this;
    }
    node = children[c];
    if (i != key.size() - 1 && (node->is_value_node_ || node->children_.size() > 1)) {
      last_val_idx = i;
    }
  }

  std::shared_ptr<TrieNode> cur;
  std::shared_ptr<TrieNode> child;
  auto new_root = root_ ? std::shared_ptr(root_->Clone()) : std::make_shared<TrieNode>();
  if (node->children_.empty()) {
    if (last_val_idx == key.size() - 1) {  // remove whole path to root
      new_root->children_.erase(key[0]);
    } else {
      for (size_t i = 0; i <= last_val_idx; i++) {
        char c = key[i];
        cur = i == 0 ? new_root : child;
        child = std::shared_ptr(cur->children_[c]->Clone());
        if (i == last_val_idx) {
          child->children_.erase(key[i + 1]);  // remove path from target node to last val node
        }
        cur->children_[c] = child;
      }
    }
  } else {
    for (size_t i = 0; i < key.size(); i++) {
      char c = key[i];
      cur = i == 0 ? new_root : child;
      if (i != key.size() - 1) {
        child = std::shared_ptr(cur->children_[c]->Clone());
      } else {
        auto value_node = cur->children_[c];  // convert `TrieNodeWithValue` to `TrieNode`
        child = std::make_shared<TrieNode>(value_node->children_);
      }
      cur->children_[c] = child;
    }
  }

  return Trie(new_root);
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
