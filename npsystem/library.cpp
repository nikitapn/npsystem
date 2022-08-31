// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_system.h"
#include "library.h"
#include "stuff.h"
#include "gccwrapperavr.h"
#include <boost/log/trivial.hpp>

struct LibDesc {
	std::unordered_set<std::string> input;
	std::unordered_set<std::string> dep;
};

static std::optional<npsys::CLibrary> CreateNPSystemLibrary(
	const std::string& libname, 
	const LibDesc& lib, 
	CGccWrapperAvr& compiler, 
	npsys::LIBRARY_TARGET target);

static void CreateDependencyTree(
	std::vector<npsys::CLibrary>& libs,
	npsys::CLibrary& lib,
	npsys::LIBRARY_TARGET target);

namespace npsys {
LibList libs;
}

std::unordered_map<std::string, LibDesc> ParseLibFile(std::string_view file) {
	if (!std::filesystem::exists(file)) {
		std::cout << "dep file not found..." << std::endl;
		return {};
	}

	struct Token {
		enum TOKEN_ID {
			SEMICOLON, COLON, WORD, COMMA, UNKNOWN, END, FB_O, FB_C
		};
		TOKEN_ID id;
		std::string attr;
		size_t line;
		size_t col;
	};

	class Lexer {
		static bool is_letter(char c) {
			return (c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z');
		}

		static bool is_digit(char c) {
			return (c >= '0' && c <= '9');
		}

		static bool is_lib_name(char c) {
			return is_letter(c) || is_digit(c) || (c == '.') || (c == '_') || (c == '-');
		}
	public:
		Lexer(std::filesystem::path file) : line_(1), col_(0), token_readed_(false) {
			std::ifstream is(file);
			is.seekg(0, is.end);
			size_t length = is.tellg();
			is.seekg(0, is.beg);

			input_.resize(length + 1);
			is.read(input_.data(), length);
			ptr_ = input_.c_str();
		}

		Token GetToken() {
			if (token_readed_) {
				token_readed_ = false;
				return readed_;
			}
			return Scan();
		}

		Token ReadToken() {
			assert(token_readed_ == false);
			token_readed_ = true;
			readed_ = Scan();
			return readed_;
		}

	private:
		std::string input_;
		const char* ptr_;
		size_t line_;
		size_t col_;
		bool token_readed_;
		Token readed_;

		Token Scan() {
			char c;
			while (c = *ptr_++) {
				col_++;
				switch (c) {
				case ' ': 
					continue;
				case '\n':
					line_++;
					col_ = 0;
					continue;
				case '\t':
					col_ += 3;
					continue;
				case ';':
					return { Token::SEMICOLON, "", line_, col_ };
				case '{':
					return { Token::FB_O, "", line_, col_ };
				case '}':
					return { Token::FB_C, "", line_, col_ };
				case ':':
					return { Token::COLON, "", line_, col_ };
				case ',':
					return { Token::COMMA, "", line_, col_ };
				default:
					if (is_letter(c)) {
						size_t pos = col_;
						std::string word;
						word.append(1, c);
						while (c = *ptr_++) {
							if (is_lib_name(c)) {
								col_++;
								word.append(1, c);
							} else {
								ptr_--;
								break;
							}
						}
						return { Token::WORD, std::move(word), line_, pos };
					}
					return { Token::UNKNOWN, " ", line_, col_ };
				}
			}
			return  { Token::END, " ", line_, col_ };
		}
	};

	class Parser {
	public:
		class ParserException : public std::runtime_error {
			using _Mybase = std::runtime_error;
		public:
			explicit ParserException(size_t line, size_t col, const std::string& _Message)
				: _Mybase(_Message.c_str()), line_(line), col_(col) {
			}
			explicit ParserException(size_t line, size_t col, const char* _Message)
				: _Mybase(_Message), line_(line), col_(col) {
			}
			size_t line() const noexcept { return line_; }
			size_t col() const noexcept { return col_; }
		private:
			size_t line_;
			size_t col_;
		};

		Parser(std::string_view file) : lex_(file) {}

		void Parse(std::unordered_map<std::string, LibDesc>& lst) {
			Token token;
			while ((token = lex_.GetToken()).id != Token::END) {
				if (token.id == Token::UNKNOWN) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"unknown token");
				}
				ParseLibDesc(token, lst);
			}
		}
	private:
		Lexer lex_;

		void ParseLibDesc(Token token, std::unordered_map<std::string, LibDesc>& lst) {
			if (token.id == Token::WORD) {
				auto lib = lst.insert({ token.attr, {} });
				if (!lib.second) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"library is already defined");
				}

				auto next = lex_.GetToken();
				if (next.id != Token::FB_O) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"there is no { after the lib declaration");
				}
				if ((next = lex_.GetToken()).id == Token::FB_C) return;
				do {
					if (next.id == Token::WORD) {
						auto col = lex_.GetToken();
						if (col.id != Token::COLON) {
							throw ParserException(token.line, token.col + token.attr.length(),
								"there is no column after the declaration");
						}
						if (next.attr == "i") ParseList(lib.first->second.input);
						if (next.attr == "d") ParseList(lib.first->second.dep);
					}
				} while ((next = lex_.GetToken()).id == Token::WORD);
				if (next.id != Token::FB_C) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"there is no } after lib declaration");
				}
			}
		}
		void ParseList(std::unordered_set<std::string>& lst) {
			auto token = lex_.GetToken();
			switch (token.id) {
			case Token::WORD:
			{
				auto next = lex_.ReadToken();
				if (next.id == Token::COMMA || next.id == Token::SEMICOLON) {
					if (!lst.insert(token.attr).second) {
						throw ParserException(token.line, token.col, "parser error");
					}
				} else {
					throw ParserException(next.line, next.col, "parser error");
				}
			}
			case Token::COMMA:
				ParseList(lst);
				break;
			case Token::SEMICOLON:
				return;
			default:
				throw ParserException(token.line, token.col, "parser error");
			}
		}
	};

	try {
		std::unordered_map<std::string, LibDesc> result;
		Parser parser(file);
		parser.Parse(result);
		return result;
	} catch (Parser::ParserException& ex) {
		std::cout << "Error in Line: " << ex.line() << " Col: " << ex.col() << " " << ex.what() << std::endl;
	}

	return {};
}

void NPSystemCompileLibraries() {
	global.ChangeCurrentDirectory(CurrentDirectory::LIBRARIES);
	if (std::filesystem::exists("lib.db")) {
		std::ifstream is("lib.db", std::ios_base::binary);
		try {
			boost::archive::binary_iarchive oa(is);
			oa >> npsys::libs;
		} catch (std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
		global.ChangeCurrentDirectory(CurrentDirectory::ROOT);
		return;
	}
	auto libs_desc = ParseLibFile("./avr/dep.txt");
	if (!libs_desc.size()) {
		global.ChangeCurrentDirectory(CurrentDirectory::ROOT);
		return;
	}
	CGccWrapperAvr gcc;
	gcc.SetPartno("atmega16");
	gcc.AppendOptions("-c -nostartfiles -nodefaultlibs -nostdlib");
	std::vector<npsys::CLibrary> libs;
	for (auto& lib : libs_desc) {
		auto r = CreateNPSystemLibrary(lib.first, lib.second, gcc, npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR);
		if (r) libs.emplace_back(std::move(r.value()));
	}
	for (auto& lib : libs) CreateDependencyTree(libs, lib, npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR);
	npsys::libs = libs;
	global.ChangeCurrentDirectory(CurrentDirectory::LIBRARIES);
	std::ofstream os("lib.db", std::ios_base::binary);
	try {
		boost::archive::binary_oarchive oa(os);
		oa << npsys::libs;
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	global.ChangeCurrentDirectory(CurrentDirectory::ROOT);
}

inline std::string remove_ext(const std::string& str) {
	size_t pos = str.find_last_of('.');
	return std::string(str.c_str(), pos);
}

std::string_view subdir (npsys::LIBRARY_TARGET target) {
	if (target == npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR) return "avr\\"sv;
	return "unknown"sv;
};

static std::optional<npsys::CLibrary> CreateNPSystemLibrary(
	const std::string& libname, 
	const LibDesc& lib, 
	CGccWrapperAvr& compiler, 
	npsys::LIBRARY_TARGET target) 
{
	npsys::CLibrary result;
	result.name = libname;
	result.additionalLibs = lib.dep;

	std::vector<std::string> ofiles;
	boost::regex expr;
	boost::smatch what;

	std::cout << "------Build started : Library : " + libname;

	std::string input;
	for (auto& i : lib.input) {
		input += "../" + i + " ";
		ofiles.push_back(remove_ext(i) + ".o");
	}

	compiler.SetDir(global.GetLibrariesDir() + std::string(subdir(target)) + libname + "\\obj");
	
	auto ok = compiler.Compile(input, std::cout);
	std::cout << std::endl;

	if (ok != 0) return {};
	
	for (auto& file : ofiles) {
		auto o = result.libobjects.insert({ file, {} });
		assert(o.second);
		o.first->second.name = file;
		auto symbols = compiler.GetSymbolTable(file, "-g");
		for (auto& sym : symbols) {
			if (sym.type == 'T') {
				o.first->second.expf.push_back(sym.name);
			}
		}
	}

	return result;
}

static void CreateDependencyTree(
	std::vector<npsys::CLibrary>& libs, 
	npsys::CLibrary& lib, 
	npsys::LIBRARY_TARGET target) 
{
	std::string input;
	std::filesystem::current_path(global.GetLibrariesDir() + std::string(subdir(target)) + lib.name + "\\obj");
	
	for (auto& l : libs) {
		if (lib.additionalLibs.find(l.name) != lib.additionalLibs.end()) {
			for (auto& o : l.libobjects) {
				input += "../../" + l.name + "/obj/" + o.first + " ";
			}
		}
	}

	for (auto& o : lib.libobjects) input += o.first + " ";

	std::ostringstream ss;
	auto ok = CreateProccessWindowlessA(CGccWrapperAvr::GetAvrToolchainPath("avr-ld") + "--cref " + input, "", ss);
	auto str = ss.str();

	std::filesystem::current_path(global.GetAppRoamingDir());
	
	boost::replace_all(str, "../../", "<");
	boost::replace_all(str, "/obj/", ">");

	// Gen Tree From Cross Table
	const char* pTable = str.c_str();
	const char* pLineBegin = pTable;
	const char* pLineEnd = pTable;

	boost::regex expr_1("^([0-9a-zA-Z_><]+)(\\s*)([0-9a-zA-Z_><]+.o)$"),
		expr_2("^(\\s*)([0-9a-zA-Z_><]+.o)$");
	boost::smatch what;

	std::string strCurrent;
	while (*pLineBegin) {
		while (*pLineEnd && (*pLineEnd) != '\n') {
			pLineEnd++;
		}
		std::string strLine(pLineBegin, pLineEnd - 1);

		if (boost::regex_search(strLine, what, expr_1)) {
			strCurrent = what[3];
		} else {
			if (boost::regex_search(strLine, what, expr_2)) {
				lib.table[what[2]].push_back(strCurrent);
			}
		}
		if (!(*pLineEnd)) break;
		pLineBegin = ++pLineEnd;
	}
/*
	for (auto& i : lib.table) {
		std::cout << i.first << ":\n";
		for (auto& j : i.second) 
			std::cout << "    " << j << "\n";
	}
	std::cout.flush();
*/
}