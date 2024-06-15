// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory
#include <iostream>

#include "../include/npcompiler/npcompiler.hpp"

//#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "ast.hpp"
#include "ast_utils.hpp"
#include "generated/lang.tab.hh"
#include "builder_llvm.hpp"

namespace npcompiler {

bool Compilation::compile(globals_resolver_cb_t glr) noexcept {
	ast::AstNode::pool_t pool{ 256 };
	ast::AstNode::pool_ = &pool;

	ast::AstNode script(ast::AstType::Module);
	ast::ParserContext parser_ctx{ glr };

	yy::set_buffer(buffer_.data(), buffer_.size());
	yy::parser parser(parser_ctx, script);
	
	bool ok = false;
	try {
		if (parser.parse() != 0 || parser_ctx.error) return false;

		BuilderLLVM builder(file_name_);

		for (auto node : ast::filter_dfs_preorder(script,
			[](ast::AstNode& n, size_t, bool) { return !n.is_expression(); })
			) {
			//std::cerr << *node << '\n';
			builder.emit(*node);
		}

		builder.print_debug();
		builder.avr_create_object();

		ok = true;
	} catch (std::exception& ex) {
		std::cerr << ex.what();
	}

	return ok;
}

} // namespace npcompiler