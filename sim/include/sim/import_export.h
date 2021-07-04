// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef __SIM_EXPORTS_H__
#define __SIM_EXPORTS_H__

#ifdef _WIN32
	#define _GLIBCXX_USE_NOEXCEPT
	#ifdef SIM_EXPORTS
		#define SIM_IMPORT_EXPORT __declspec(dllexport)
	#else
		#define SIM_IMPORT_EXPORT __declspec(dllimport)
	#endif
#else
	#define SIM_IMPORT_EXPORT 
#endif

#endif // __SIM_EXPORTS_H__