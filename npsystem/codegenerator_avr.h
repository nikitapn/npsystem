// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "gccwrapperavr.h"
#include "blockdecl.h"
#include "codegenerator.h"

namespace npsys {
class CAlgorithmExt;
}

enum class OP_TYPE {
	GCC_OP_R,
	GCC_OP_W,
	GCC_OP_RW
};

class AsmBlock {
	friend std::stringstream& operator << (std::stringstream& ss, const AsmBlock& block);

	struct Operand {
		OP_TYPE type;
		const std::string name;
		const npsys::variable* var;
	};
	//
	const std::string& block_id_;
	
	std::deque<Operand> m_ops;
	std::stringstream m_body;
	std::string m_clobbers;

	std::tuple<std::string, std::string, std::string> Normalize() const noexcept;
public:
	template<typename... Args>
	void AddLabel(Args&&... args) {
		m_body << "\t\t\"" << block_id_;
		(m_body << ... << args) << ": \\n\\t\"\n";
	}

	template<typename... Args>
	void AddCmd(Args&&... args) {
		m_body << "\t\t\"";
		(m_body << ... << args) << " \\n\\t\"\n";
	}

	template<typename Instruction, typename Label>
	void AddBranch(Instruction&& instruction, Label&& label) {
		m_body << "\t\t\"" << instruction << ' ' << block_id_ << label << " \\n\\t\"\n";
	}
	
	void AddOperand(OP_TYPE type, const std::string& name, const npsys::variable* var);
	void AddClobber(const std::string& r);

	explicit AsmBlock(const std::string& block_id) 
		: block_id_(block_id) 
	{
	}
};

class CAVR5CodeGenerator
	: public CCodeGenerator
{
protected:
	npsys::algorithm_n& alg_;
	const avrinfo::FirmwareInfo& info_;
	size_t conv_bit_n_ = 0;

	void AddLibrary(std::string lib);
	void PasteCode(const std::string& code) noexcept {
		m_code << "\t" << code << "\n";
	}
	std::string CastBit(const npsys::variable* var) noexcept;
	std::string CastQBit(const npsys::variable* var) noexcept;
public:
	std::unique_ptr<CGccWrapperAvr> gcc_;
	std::unordered_set<std::string> libraries;
	
	CAVR5CodeGenerator(npsys::algorithm_n alg, const avrinfo::FirmwareInfo& info);
	void SaveToFile();
	virtual void Reset();
#define AAF(x) virtual void Generate(x*);
	ALPHA_BLOCKS()
#undef AAF
};
