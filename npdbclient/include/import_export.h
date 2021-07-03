#pragma once

#ifdef _WIN32
#	ifdef NPDBCLIENT_EXPORTS 
#		define NPDB_IMPORT_EXPORT __declspec(dllexport)
#	else
#		define NPDB_IMPORT_EXPORT __declspec(dllimport)
#	endif
#else 
#	define NPDB_IMPORT_EXPORT
#endif