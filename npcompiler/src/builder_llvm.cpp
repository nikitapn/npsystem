// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "builder_llvm.hpp"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <map>

#pragma comment(lib, "LLVMCore.lib")
#pragma comment(lib, "LLVMCodeGen.lib")
#pragma comment(lib, "LLVMSupport.lib")
#pragma comment(lib, "LLVMRemarks.lib")
#pragma comment(lib, "LLVMBitstreamReader.lib")
#pragma comment(lib, "LLVMBinaryFormat.lib")

namespace npcompiler {

class BuilderLLVMImpl {
  std::unique_ptr<llvm::LLVMContext> ctx_;
  std::unique_ptr<llvm::Module> module_;
	std::unique_ptr<llvm::IRBuilder<>> builder_;
public:
	llvm::Value* make_constant(ast::AstNode& n) {
		assert(ast::is_number(n));
		if (n.llvm_value) return n.llvm_value;
		
		auto& num = n.as_number();
		if (num.as_floating_point) {
			n.llvm_value = llvm::ConstantFP::get(*ctx_.get(), llvm::APFloat(num.value_float));
		} else {
			n.llvm_value = llvm::ConstantInt::get(*ctx_.get(), llvm::APInt(32, num.value_int, true));
		}
	}

	/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
	/// the function.  This is used for mutable variables etc.
	llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,
		const std::string& VarName) {
		llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
		return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*ctx_.get()), 0, VarName.c_str());
	}

	void emit(ast::AstNode& n) {
		using enum ast::AstType;
		switch (n.type) {
		case Identifier:

			break;
		case VarDeclSeq:
		{
			for (auto& child : n) {
				if (child->type == Assignment) {
					std::cerr << child->node_type_str() << " ";
				} else if (child->type == Identifier) {
					//builder_->CreateLoad(
					std::cerr << child->node_type_str() << " ";
				}
			}
			std::cerr << "\n";
			break;
		}
		case ast::AstType::StmtList:
			std::cerr << n.node_type_str() << "\n";
			break;
		case ast::AstType::Assignment:
			std::cerr << n.node_type_str() << "\n";
			break;
		case ast::AstType::Number:
			std::cerr << n.node_type_str() << "\n";
			break;
		default:
			using namespace std::string_literals;
			throw std::runtime_error("Builder not implemented: "s.append(n.node_type_str().data(), n.node_type_str().size()));
		}
	}

	BuilderLLVMImpl() {
		// Open a new context and module.
		ctx_ = std::make_unique<llvm::LLVMContext>();
		module_ = std::make_unique<llvm::Module>("my cool jit", *ctx_);

		// Create a new builder for the module.
		builder_ = std::make_unique<llvm::IRBuilder<>>(*ctx_);
	}
};

void BuilderLLVM::emit(ast::AstNode& n) { impl_->emit(n); }

BuilderLLVM::BuilderLLVM() {
	impl_ = new BuilderLLVMImpl();
}

BuilderLLVM::~BuilderLLVM() {
	delete impl_;
}

} // namespace npcompiler
