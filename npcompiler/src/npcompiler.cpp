// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "../include/npcompiler/npcompiler.hpp"

#include "ast.hpp"
#include "generated/lang.tab.hh"
#include "builder_llvm.hpp"

namespace npcompiler {

bool Compilation::compile() noexcept {
	ast::AstNode script(ast::non_term, ast::AstType::Script);
	ast::ParserContext parser_ctx;

	yy::set_buffer(buffer_.data(), buffer_.size());
	yy::parser parser(parser_ctx, script);
	
	try {
		if (parser.parse() != 0) return false;
		
		BuilderLLVM builder;

		for (auto node : ast::dfs_preorder(script)) {
			//std::cerr << node->node_type_str() << "\n";
			builder.emit(*node);
		}
		

		return true;
	} catch (std::exception& ex) {
		std::cerr << ex.what();
	}

	return false;
}

} // namespace npcompiler