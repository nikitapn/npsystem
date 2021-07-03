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