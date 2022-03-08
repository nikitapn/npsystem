// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <string>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#	ifdef NPCOMPILER_EXPORTS 
#		define NPCOMPILER_IMPORT_EXPORT __declspec(dllexport)
#	else
#		define NPCOMPILER_IMPORT_EXPORT __declspec(dllimport)
#	endif
#else 
#	define NPDB_IMPORT_EXPORT
#endif

namespace npcompiler {

class Compilation {
	std::string buffer_;
public:
	NPCOMPILER_IMPORT_EXPORT bool compile() noexcept;
	
	Compilation(std::string_view source) noexcept {
		buffer_.assign(source);
		buffer_.append(1, '\0'); 
		buffer_.append(1, '\0');
	}

	Compilation(std::filesystem::path file) {
		std::ifstream ifs(file);

		if (!ifs) throw std::runtime_error("couldn't open \"" + file.string() + "\"");
	
		buffer_.assign(std::istreambuf_iterator<char>(ifs), {});
		buffer_.append(1, '\0'); 
		buffer_.append(1, '\0');
	}
};

} // namespace npcompiler