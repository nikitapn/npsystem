#pragma once

#include "memorymanager.h"


struct Symbol {
	int addr;
	char type;
	std::string name;
};

class CGccWrapperAvr
{
public:
	static std::string GetAvrToolchainPath(const std::string& exe);
	int Compile(const std::string& stInput, std::ostream& out);
	int Link(const std::string& stInput, const std::string& stOutput, std::ostream& out);
	int GetSize(const std::string& file);
	void CreateLss(const std::string& stElf);
	void CreateHex(const std::string& stElf);
	std::vector<Symbol> GetSymbolTable(const std::string& file, std::string options = "");
	void AppendOptions(const std::string& stOpt) noexcept  {
		m_stOptions += stOpt + " ";
	}
	void SetPartno(const char* partno) noexcept {
		m_stMC = "-mmcu=";
		m_stMC += partno;
		m_stMC += ' ';
	}
	void AppendInclude(const std::string& stInclude) noexcept {
		m_stInclude += "-I" + stInclude + " ";
	}
	void Reset() noexcept  {
		m_stInclude.clear();
		m_stOptions.clear();
		m_stMC.clear();
	}
	void SetDir(const std::string& dir) noexcept {
		m_stBuildDir = dir;
	}
	void AddVariable(const npsys::variable* var);
	std::string GetVarDecl();
	void CreateAlgorithmLinkerScript(int origin, const avrinfo::FirmwareInfo& info);
	void CreateLinkerScript(int origin);
private:
	std::string m_stMC;
	std::string m_stOptions;
	std::string m_stInclude;
	std::string m_stBuildDir;
	static const char m_linker_script[];
	static const char m_linker_script_default[];
	std::map<int, std::string> m_no_overlap_v;
	std::vector<const npsys::variable*> m_variables;
	std::set<std::string> m_functions;
};