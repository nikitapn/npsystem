// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "vm.h"
#include <vector>

namespace sim {

enum class Instruction {
	Load, Store, Add, Sub, Mul, Div, Print
};

class VM::Impl {
	static constexpr auto initial_code_size = 1024;
	static constexpr auto initial_stack_size = 512;
public:
	Impl();
	void run();
private:
	std::vector<uint32_t> code;
	std::vector<uint32_t> stack;
	size_t ip = 0;
	size_t sp = 0;
	bool running_ = false;
};


VM::Impl::Impl() {
	code.resize(initial_code_size);
	stack.resize(initial_stack_size);
}


void VM::Impl::run() {
	if (!running_) return;

	Instruction ist = static_cast<Instruction>(code[ip]);

	switch (ist) {
	case Instruction::Load:
		break;
	case Instruction::Store:
		break;
	default:
		assert(false);
		break;
	}
}

SIM_IMPORT_EXPORT
VM::VM() {
	impl_ = std::make_unique<Impl>();
}

SIM_IMPORT_EXPORT
VM::~VM() {

}

SIM_IMPORT_EXPORT
void VM::run() {
	impl_->run();
}

} //namespace sim