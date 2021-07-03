#include "stdafx.h"
#include "Global.h"
#include "GccWrapperAvr.h"
#include <avr_firmware/net.h>
#include "config.h"

const char CGccWrapperAvr::m_linker_script[] = "\
OUTPUT_FORMAT(\"elf32-avr\",\"elf32-avr\",\"elf32-avr\")\n\
OUTPUT_ARCH(avr:5)\n\
__TEXT_REGION_LENGTH__ = DEFINED(__TEXT_REGION_LENGTH__) ? __TEXT_REGION_LENGTH__ : 128K;\n\
MEMORY\n\
{\n\
	text (rx) : ORIGIN = 0x%.4x, LENGTH = __TEXT_REGION_LENGTH__\n\
}\n\
SECTIONS\n\
{\n\
 .text : \n\
 {\n\
    *(.text)\n\
	. = ALIGN(2);\n\
    *(.text.*)\n\
	. = ALIGN(2);\
  } > text\n\
";

void CGccWrapperAvr::CreateAlgorithmLinkerScript(int origin, const avrinfo::FirmwareInfo& info) {
	std::ofstream ls_file(m_stBuildDir + "\\ls.x");
	ls_file << string_format(m_linker_script, origin);
	
	std::map<int, std::string> v;

	for (auto& i : m_no_overlap_v) {
		const char* p = i.second.c_str();
		ls_file << string_format(
			"\t.%s 0x%.4x : {KEEP(*(.%s)); }\n",
			p, i.first | 0x800000, p);
	}

	int addr = info.rmem.info + offsetof(net::RuntimeInfoController, u32_time);
	ls_file << string_format(
		"\t.%s 0x%.4x : {KEEP(*(.%s)); }\n",
		"dwCnt", addr | 0x800000, "dwCnt");
	//

	ls_file << "}\n";
	ls_file.close();
}

void CGccWrapperAvr::CreateLinkerScript(int origin) {
	std::ofstream ls_file(m_stBuildDir + "\\ls.x");
	ls_file << string_format(m_linker_script, origin);
	ls_file << "}\n";
	ls_file.close();
}

std::string CGccWrapperAvr::GetAvrToolchainPath(const std::string& exe) {
		return "\"" + g_cfg.toolchain_dir + "\\avr8\\avr8-gnu-toolchain\\bin\\" + exe + "\" ";
}

int CGccWrapperAvr::Compile(const std::string& stInput, std::ostream& out) {
	return CreateProccessWindowlessA(
		GetAvrToolchainPath("avr-gcc") + "-Os -Wall " + m_stMC + m_stInclude + m_stOptions + stInput,
		m_stBuildDir, 
		out);
}

int CGccWrapperAvr::Link(const std::string& stInput, const std::string& stOutput, std::ostream& out) {
	std::string cmd = GetAvrToolchainPath("avr-gcc") + 
		"-Os -Wall " + m_stMC + m_stInclude + m_stOptions + stInput + " -o " + stOutput;
	return CreateProccessWindowlessA(cmd, m_stBuildDir, out);
}

int CGccWrapperAvr::GetSize(const std::string& file) {
	
	std::stringstream out;
	auto cmd = GetAvrToolchainPath("avr-size") + file;
	CreateProccessWindowlessA(cmd, m_stBuildDir, out);
	out << '\n';
	auto str = out.str();

	const char* second_line = str.c_str() + str.find('\n');

	int size;
	sscanf_s(second_line, "%d", &size);

	return size;
}

void CGccWrapperAvr::CreateLss(const std::string& elf) {
	auto lss = elf;
	boost::replace_all(lss, "elf", "lss");
	
	std::ofstream file(global.GetBuildDir() + lss, std::ofstream::out | std::ofstream::binary);
	CreateProccessWindowlessA(GetAvrToolchainPath("avr-objdump") + "-dS " + elf, m_stBuildDir, file);
	file << '\n';
}

void CGccWrapperAvr::CreateHex(const std::string& stElf) {
	std::string cmd, hex = stElf;
	boost::replace_all(hex, "elf", "hex");
	cmd = GetAvrToolchainPath("avr-objcopy") + "-j .text -j .progmem -O ihex " + stElf + " " + hex;
	CreateProccessWindowlessA(cmd, m_stBuildDir);
}

std::vector<Symbol> CGccWrapperAvr::GetSymbolTable(const std::string& file, std::string options) {
	std::vector<Symbol> symbols;
	std::string cmd = GetAvrToolchainPath("avr-nm") + options + " " + file;
	std::stringstream ss;

	CreateProccessWindowlessA(cmd, m_stBuildDir, ss);
	
	std::string line;
	boost::smatch what;

	boost::regex expr("(\\S*)(\\s+)(\\S)(\\s+)(\\S+)");

	while (std::getline(ss,line)) {
		if (boost::regex_search(line, what, expr)) {
			const std::string& addr = what[1];
			const std::string& type = what[3];
			const std::string& symbol = what[5];
			symbols.push_back({ nplib::format::from_hex(addr), type[0], symbol });
		}
	}

	return symbols;
}

std::string CGccWrapperAvr::GetVarDecl() {
	std::string decl;
	decl.reserve(128);
	
	for (auto const var_ptr : m_variables) {
		std::string var = var_ptr->to_vardecl();
		auto it = m_no_overlap_v.find(var_ptr->GetAddr());
		if (it != m_no_overlap_v.end()) {
			decl += "#define " + var + " " + it->second + "\n";
			if (!var_ptr->IsBit() && var_ptr->IsQuality()) {
				decl += "#define " + var + "_q " + it->second + "_q" + "\n";
			}
		} else {
			decl += "static " + var_ptr->to_ctype() + " " + var + " __attribute__((section(\"." + var + "\"))) __attribute__((__used__));\n";
			m_no_overlap_v[var_ptr->GetAddr()] = var;
			
			if (!var_ptr->IsBit() && var_ptr->IsQuality()) {
				m_no_overlap_v[var_ptr->GetAddr() + var_ptr->GetSize()] = var + "_q";
				decl += "static uint8_t " + var + "_q __attribute__((section(\"." + var + "_q\"))) __attribute__((__used__));\n";
			}
		}
	}
	return decl;
}

void CGccWrapperAvr::AddVariable(const npsys::variable* var) {
	auto begin = m_variables.begin();
	auto end = m_variables.end();
	
	if (std::find(begin, end, var) != m_variables.end()) return;

	bool inserted = false;
	for (auto it = begin; it != end; ++it) {
		if ((*it)->self_node.id() > var->self_node.id()) {
			m_variables.insert(it, var);
			inserted = true;
			break;
		}
	}
	
	if (!inserted) {
		m_variables.insert(end, var);
	}
}