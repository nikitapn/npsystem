// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "ast.hpp"
#include <memory>
#include <array>
#include <functional>
#include <algorithm>
#include <fstream>

template<typename Fn>
struct OstreamWrapper {
		Fn fn;
};

template<typename Fn>
inline std::ostream& operator<<(std::ostream& os, const OstreamWrapper<Fn>& wrapper) {
	wrapper.fn(os);
	return os;
}

class BlockDepth {
	friend std::ostream& operator<<(std::ostream&, const BlockDepth&);
	size_t depth_ = 0;
public:

	std::string str() const noexcept {
		return std::to_string(depth_);
	}

	BlockDepth& operator =(size_t n) {
		depth_ = n;
		return *this;
	}

	BlockDepth& operator --() {
		// assert(depth_);
		if (depth_ == 0) depth_ = 1;
		--depth_;
		return *this;
	}

	BlockDepth& operator ++() {
		++depth_;
		return *this;
	}

	BlockDepth operator ++(int) {
		BlockDepth tmp = *this;
		++depth_;
		return tmp;
	}

	BlockDepth operator +(size_t depth) {
		BlockDepth tmp = *this;
		tmp.depth_ += depth;
		return tmp;
	}

	BlockDepth operator -(size_t depth) {
		assert(depth_ > depth);
		BlockDepth tmp = *this;
		tmp.depth_ -= depth;
		return tmp;
	}

	BlockDepth& operator -=(size_t depth) {
		assert(depth_ > depth);
		depth_ -= depth;
		return *this;
	}

	BlockDepth& operator +=(size_t depth) {
		depth_ += depth;
		return *this;
	}
};

inline std::ostream& operator<<(std::ostream& os, const BlockDepth& block) {
	for (size_t i = 0; i < block.depth_; ++i) os << "  ";
	return os;
}

class Builder {
protected:
	Context& ctx_;
	BlockDepth block_depth_;
	bool always_full_namespace_ = false;

	void always_full_namespace(bool flag) { always_full_namespace_ = flag; }
	void make_arguments_structs(AstFunctionDecl* fn);
	void emit_arguments_structs(std::function<void(AstStructDecl*)> fn);

	auto bb(bool newline = true) {
		return OstreamWrapper{[this, newline](std::ostream& os) {
			if (newline)
				os << block_depth_ << "{\n";
			++block_depth_;
		}};
	}

	auto eb(bool newline = true) {
		return OstreamWrapper{[this, newline](std::ostream& os) {
			--block_depth_;
			if (newline)
				os << block_depth_ << "}\n";
		}};
	}

	auto bl() {
		return OstreamWrapper{[this](std::ostream& os) {
			os << block_depth_;
		}};
	}

public:
	virtual void emit_constant(const std::string& name, AstNumber* number) = 0;
	virtual void emit_struct(AstStructDecl* s) = 0;
	virtual void emit_exception(AstStructDecl* s) = 0;
	virtual void emit_namespace_begin() = 0;
	virtual void emit_namespace_end() = 0;
	virtual void emit_interface(AstInterfaceDecl* ifs) = 0;
	virtual void emit_file_footer() = 0;
	virtual void emit_using(AstAliasDecl* u) = 0;
	virtual void emit_enum(AstEnumDecl* e) = 0;
	
	Builder(Context& ctx) : ctx_(ctx) {}
	virtual ~Builder() = default;
};

class BuildGroup {
	Context& ctx_;
	size_t size_;
	static constexpr size_t N = 2;
	std::array<std::unique_ptr<Builder>, N> builders_;
public:
	template<typename F, typename... Args>
	void emit(F fptr, Args&&... args) {
		auto mf = std::mem_fn(fptr);
		std::for_each(builders_.begin(), builders_.begin() + size_, 
			[&](auto& ptr) { mf(ptr.get(), std::forward<Args>(args)...); }
		);
	}

	template<typename T, typename... Args>
	void add(Args&&... args) {
		assert(size_ < N);
		builders_[size_++] = std::make_unique<T>(ctx_, std::forward<Args>(args)...);
	}

	BuildGroup(Context& ctx) : ctx_{ ctx }, size_{ 0 } {}
};
