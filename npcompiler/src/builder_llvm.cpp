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
	/*
	Value* make_constant(ast::AstNode& n) {
		auto& number = n.cast<ast::Number>();
		auto& llvm_value = number.llvm_value;

		if (llvm_value) return llvm_value;

		llvm_value = number.is_float()
			? ConstantFP::get(*ctx_.get(), APFloat((float)number))
			: llvm_value = ConstantInt::get(*ctx_.get(), APInt(32, (int)number, true));
	}*/

	llvm::Type* to_llvm_type(int var_type) {
		switch (npsys::variable::GetClearType(var_type))
		{
		case fl::FDType::NPT_BOOL:
			return Type::getInt1Ty(*ctx_);
			break;

		case fl::FDType::NPT_I8:
		case fl::FDType::NPT_U8:
			return Type::getInt8Ty(*ctx_);

		case fl::FDType::NPT_I16:
		case fl::FDType::NPT_U16:
			return Type::getInt16Ty(*ctx_);

		case fl::FDType::NPT_I32:
		case fl::FDType::NPT_U32:
			return Type::getInt32Ty(*ctx_);
		
		case fl::FDType::NPT_F32:
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
		using npsys::variable;
		using enum ast::AstType;

		assert(n.is_expression());

		switch (n.type) {

		case Add:
			return mirb_->CreateAdd(emit_expression(n[0]), emit_expression(n[1]));

		case Number: {
			auto& number = n.cast<ast::Number>();
			if (!number.is_float()) {
				auto& i = number.as_int();
				auto [size, sign] = fl::get_number_info(i.type);
				return ConstantInt::get(*ctx_, APInt(size, i.x, sign));
			} else {
				return ConstantFP::get(*ctx_, APFloat((float)number));
			}
		}

		case Identifier: {
			auto& ident = n.cast<ast::Identifier>();
			return mirb_->CreateLoad(to_llvm_type(ident.type), ident.llvm_value, ident.name);
		}

		case Cast: {
			auto value = emit_expression(n[0]);
			auto from = n[0].get_expression_type();
			auto to = n.cast<ast::CastTo>().type;

			std::cerr << "Casting: " << variable::to_ctype(from) << " " << n[0] << " -> " << variable::to_ctype(to) << '\n';

			assert(from != to);

			// Cast operators ...
			// HANDLE_CAST_INST(38, Trunc   , TruncInst   )  // Truncate integers
			// HANDLE_CAST_INST(39, ZExt    , ZExtInst    )  // Zero extend integers
			// HANDLE_CAST_INST(40, SExt    , SExtInst    )  // Sign extend integers
			// HANDLE_CAST_INST(41, FPToUI  , FPToUIInst  )  // floating point -> UInt
			// HANDLE_CAST_INST(42, FPToSI  , FPToSIInst  )  // floating point -> SInt
			// HANDLE_CAST_INST(43, UIToFP  , UIToFPInst  )  // UInt -> floating point
			// HANDLE_CAST_INST(44, SIToFP  , SIToFPInst  )  // SInt -> floating point
			// HANDLE_CAST_INST(45, FPTrunc , FPTruncInst )  // Truncate floating point
			// HANDLE_CAST_INST(46, FPExt   , FPExtInst   )  // Extend floating point
			// HANDLE_CAST_INST(47, PtrToInt, PtrToIntInst)  // Pointer -> Integer
			// HANDLE_CAST_INST(48, IntToPtr, IntToPtrInst)  // Integer -> Pointer
			// HANDLE_CAST_INST(49, BitCast , BitCastInst )  // Type cast
			// HANDLE_CAST_INST(50, AddrSpaceCast, AddrSpaceCastInst)  // addrspace cast

			if (fl::is_integer(from) && fl::is_integer(to)) {
				return mirb_->CreateIntCast(value, to_llvm_type(to), fl::is_signed(from));
			} else if (fl::is_float(to)) {
				//mirb_->CreateIntrinsic(
				//mirb_->get
			}


			break;
		}

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
				auto& ident = var->cast<ast::Identifier>();
				ident.llvm_value = CreateEntryBlockAlloca(cur_fun_, ident.name, ident.type);
			}
			break;

		case StmtList:
			break;

		case Assignment: {
			// 0 - ident, 1 - expr
			auto& ident = n[0].cast<ast::Identifier>();
			auto& expr = n[1];
			mirb_->CreateStore(emit_expression(expr), ident.llvm_value);
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

	static constexpr auto sss = sizeof(ast::AstNode);

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
			std::cerr << "AVR target not found: " << EC << '\n';
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

