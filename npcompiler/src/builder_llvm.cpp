// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "builder_llvm.hpp"

#include <map>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/LegacyPassManager.h>
//#include <llvm/Linker/Linker.h>
#include <llvm/Bitcode/BitcodeWriter.h>
				 
#include <llvm/ExecutionEngine/ExecutionEngine.h>
//#include <llvm/ExecutionEngine/GenericValue.h>
//#include <llvm/ExecutionEngine/MCJIT.h>

//#include <llvm/ExecutionEngine/Orc/Core.h>
//#include <llvm/ExecutionEngine/Orc/Mangling.h>

#include  <llvm/MC/MCContext.h>
#include  <llvm/MC/MCSymbol.h>


#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/MC/TargetRegistry.h>

#include <llvm/Support/FileSystem.h> // raw_fd_ostream
#include <npsys/variable.h>

using namespace llvm;

namespace npcompiler {

class BuilderLLVMImpl {
  std::unique_ptr<LLVMContext> ctx_;
  std::unique_ptr<Module> module_;
	std::unique_ptr<IRBuilder<>> mirb_;
	Function* cur_fun_;
public:
	Value* make_constant(ast::AstNode& n) {
		assert(ast::is_number(n));
		if (n.llvm_value) return n.llvm_value;
		
		auto& num = n.as_number();
		if (num.as_floating_point) {
			n.llvm_value = ConstantFP::get(*ctx_.get(), APFloat(num.value_float));
		} else {
			n.llvm_value = ConstantInt::get(*ctx_.get(), APInt(32, num.value_int, true));
		}
	}

	llvm::Type* to_llvm_type(int var_type) {
		switch (npsys::variable::GetClearType(var_type))
		{
		case npsys::variable::VT_UNDEFINE:
			assert(false);
			return nullptr;
			break;

		case npsys::variable::VT_DISCRETE:
			return Type::getInt1Ty(*ctx_);
			break;

		case npsys::variable::VT_BYTE:
		case npsys::variable::VT_SIGNED_BYTE:
			return Type::getInt8Ty(*ctx_);

		case npsys::variable::VT_WORD:
		case npsys::variable::VT_SIGNED_WORD:
			return Type::getInt16Ty(*ctx_);

		case npsys::variable::VT_DWORD:
		case npsys::variable::VT_SIGNED_DWORD:
			return Type::getInt32Ty(*ctx_);

		case npsys::variable::VT_FLOAT:
			return Type::getFloatTy(*ctx_);

		default:
			assert(false);
			return nullptr;
		}
	}

	/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
	/// the function.  This is used for mutable variables etc.
	AllocaInst* CreateEntryBlockAlloca(
		Function* function,
		const std::string& var_name,
		int var_type
	) {
		IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
		return TmpB.CreateAlloca(to_llvm_type(var_type), 0, var_name.c_str());
	}

	Value* emit_expression(ast::AstNode& n) {
		assert(n.is_expression());

		//BitCastInst::
		//CastInst::CreateIntegerCast

		//builder_->create

		//mirb_->CreateBitCast()

		using enum ast::AstType;

		switch (n.type) {

		case Add:
			return mirb_->CreateAdd(emit_expression(n[0]), emit_expression(n[1]));
		
		case Number: {
			auto number = n.as_number();
			if (!number.as_floating_point)
				return ConstantInt::get(*ctx_, APInt(16, number.value_int, true));
			else 
				return ConstantFP::get(*ctx_, APFloat(number.value_float));
		}

		case Identifier:
			return mirb_->CreateLoad(to_llvm_type(n.as_ident().type), n.as_ident().ptr, n.as_ident().name);
			break;

		default:
			using namespace std::string_literals;
			throw std::runtime_error("Unknown expression: "s.append(n.node_type_str().data(), n.node_type_str().size()));
		};

		return nullptr;
	}

	void emit(ast::AstNode& n) {
		assert(!n.is_expression());

		using enum ast::AstType;

//		std::cerr << n.node_type_str() << '\n';

		switch (n.type) {

		case Program: {
			cur_fun_ =
				Function::Create(FunctionType::get(Type::getVoidTy(*ctx_), false), 
					GlobalValue::LinkageTypes::ExternalLinkage, 
					"program", 
					*module_
				);
			
			// Create a new basic block to start insertion into.
			auto bb = BasicBlock::Create(*ctx_, "program", cur_fun_);
			mirb_->SetInsertPoint(bb);

			break;
		}

		case Program_End:
			mirb_->CreateRetVoid();
			// Validate the generated code, checking for consistency.
			verifyFunction(*cur_fun_);
			break;

		case Function:
		case FunctionBlock:
			break;

		case LocalVarDeclSeq:
			for (auto var : n) {
				auto& ident = var->as_ident();
				ident.ptr = CreateEntryBlockAlloca(cur_fun_, ident.name, ident.type);
			}
			break;

		case StmtList:
			break;

		case ast::AstType::Assignment: {
			// 0 - ident, 1 - expr
			mirb_->CreateStore(emit_expression(n[1]), n[0].as_ident().ptr);
			break;
		}
		default:
			using namespace std::string_literals;
			throw std::runtime_error("Builder not implemented: "s.append(n.node_type_str().data(), n.node_type_str().size()));
		}
	}

	void print_debug() {
		// Print out all of the generated code.
		module_->print(errs(), nullptr);

		/*
		auto& mod = *module_;

		auto eb = EngineBuilder(std::move(module_));
		auto native_target = eb.selectTarget();
		
		if (!native_target) {
			errs() << "Error: native target undefined.\n";
			abort();
		}
		
		auto ee = eb.create(native_target);
		if (!ee) {
			errs() << "Error: execution engine creation failed.\n";
			abort();
		}
		std::vector<GenericValue> args;
		Function* brainf_func = mod.getFunction("program");
		GenericValue gv = ee->runFunction(brainf_func, args);
		// Genereated code calls putchar, and output is not guaranteed without fflush.
		// The better place for fflush(stdout) call would be the generated code, but it
		// is unmanageable because stdout linkage name depends on stdlib implementation.
		fflush(stdout);
		*/

		//std::error_code ec;
		//raw_fd_ostream ofs(StringRef{ "D:/temp/llvm/test.elf" }, ec);

		//WriteBitcodeToFile(*module_, ofs);
	}

	BuilderLLVMImpl(std::string_view module_name) {
		// Open a new context and module.
		ctx_ = std::make_unique<LLVMContext>();
		module_ = std::make_unique<Module>(StringRef{ module_name.data(), module_name.length() }, *ctx_);

		// Create a new builder for the module.
		mirb_ = std::make_unique<IRBuilder<>>(*ctx_);
	}

	void avr_create_object() {
		//llvm::InitializeNativeTarget();
		//LLVMInitializeNativeAsmPrinter();

		LLVMInitializeAVRTargetInfo();
		LLVMInitializeAVRTarget();
		LLVMInitializeAVRTargetMC();
		LLVMInitializeAVRAsmPrinter();

		LLVMInitializeARMTargetInfo();
		LLVMInitializeARMTarget();
		LLVMInitializeARMTargetMC();
		LLVMInitializeARMAsmPrinter();

		auto triple = Triple("avr", "", "", "elf").normalize();
		std::string EC;
		auto avr_target = TargetRegistry::lookupTarget(triple, EC);
		if (!avr_target) {
			std::cerr << "AVR targer not found: " << EC << '\n';
			std::abort();
		}
		
		auto CPU = "avr4";
		auto Features = "";

		TargetOptions opt;
		auto RM = Optional<Reloc::Model>();
		auto avr_target_machine = avr_target->createTargetMachine(triple, CPU, Features, opt, RM);

		assert(avr_target_machine);

		module_->setDataLayout(avr_target_machine->createDataLayout());
		module_->setTargetTriple(triple);

		std::error_code ec;
		raw_fd_ostream dest("D:/temp/llvm/test.elf", ec, sys::fs::OF_None);

		if (ec) {
			errs() << "Could not open file: " << ec.message();
			std::abort();
		}

		legacy::PassManager pass;
		auto FileType = CGFT_AssemblyFile;
	
		if (avr_target_machine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
			errs() << "TargetMachine can't emit a file of this type";
			std::abort();
		}

		pass.run(*module_);
		dest.flush();
	}

}; // class BuilderLLVM

void BuilderLLVM::emit(ast::AstNode& n) { impl_->emit(n); }
void BuilderLLVM::print_debug() { impl_->print_debug(); }
void BuilderLLVM::avr_create_object() { impl_->avr_create_object(); }


BuilderLLVM::BuilderLLVM(std::string_view module_name) { 
	impl_ = new BuilderLLVMImpl(module_name); 
}

BuilderLLVM::~BuilderLLVM() {
	delete impl_;
}

} // namespace npcompiler
