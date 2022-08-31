#pragma once

#include "ast.hpp"

#include <coroutine>
#include <experimental/generator>

namespace npcompiler::ast {
using std::experimental::generator;

namespace impl {

template<bool use_filter, typename... Func>
generator<AstNode*> filter_dfs_preorder(AstNode& n, Func&&... fns) {
	std::stack<std::pair<AstNode::children_t::iterator, AstNode::children_t::iterator>> stack;
	stack.push({n.begin(), n.end()});

	do {
		auto& cur = stack.top();
		if (cur.first != cur.second) {
			auto cur_ptr = *cur.first++;
			if constexpr (use_filter) {
				if ((... || fns(*(cur_ptr), stack.size(), cur.first == cur.second))) {
					co_yield cur_ptr;
				}
			} else {
				co_yield cur_ptr;
			}
			stack.push({cur_ptr->begin(), cur_ptr->end()});
			} else {
			stack.pop();
		}
	} while (!stack.empty());
}

inline void tree_height_r(const AstNode* n, size_t height, size_t& max_height) {
	if (max_height < height) max_height = height;
	for (auto& c : *n) tree_height_r(c, ++height, max_height);
}

} // namespace impl

template<typename... Func>
generator<AstNode*> filter_branch_discard_dfs_preorder(AstNode& n, Func&&... fns) {
	std::stack<std::pair<AstNode::children_t::iterator, AstNode::children_t::iterator>> stack;
	stack.push({ n.begin(), n.end() });

	do {
		auto& cur = stack.top();
		if (cur.first != cur.second) {
			auto cur_ptr = *cur.first++;
			if ((... || fns(*(cur_ptr), stack.size() - 1))) {
				co_yield cur_ptr; // return top tree and discard sub trees
			} else {
				stack.push({ cur_ptr->begin(), cur_ptr->end() });
			}
		} else {
			stack.pop();
		}
	} while (!stack.empty());
}

inline generator<AstNode*> dfs_preorder(AstNode& n) {
	return impl::filter_dfs_preorder<false>(n);
}

template<typename... Func>
generator<AstNode*> filter_dfs_preorder(AstNode& n, Func&&... fns) {
	return impl::filter_dfs_preorder<true>(n, std::forward<Func>(fns)...);
}

template<typename... Func>
generator<std::pair<AstNode*, size_t>> bfs_preorder(AstNode& n) {
	std::queue<std::pair<AstNode*, size_t>> que;
	que.emplace(&n, 0);

	do {
		auto& cur = que.front();
		const size_t next_level = cur.second + 1;
		for (auto& c : *cur.first) {
			 que.emplace(c, next_level);
		}
		
		co_yield cur;
		
		que.pop();
	} while (!que.empty());
}

template<typename... Func>
void traverse_postorder(AstNode* n, Func&&... fns) {
	for (auto& c : *n) traverse_postorder(c, fns...);
	(fns(n), ...);
}

inline size_t tree_height(const AstNode& n) {
	size_t height = 0;
	impl::tree_height_r(&n, height, height);
	return height;
}

} // namespace ast