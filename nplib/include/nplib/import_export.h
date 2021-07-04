// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifdef _WIN32
#	ifdef NPLIB_EXPORTS 
#		define NPLIB_IMPORT_EXPORT __declspec(dllexport)
#	else
#		define NPLIB_IMPORT_EXPORT __declspec(dllimport)
#	endif
#else
#	define NPLIB_IMPORT_EXPORT
#endif