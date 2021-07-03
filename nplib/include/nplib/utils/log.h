#pragma once

#ifdef _MSC_VER

#include <string>
#include <fstream>
#include "../import_export.h"

enum LOG_MSG_TYPE {
	LOG_MSG_ERROR,
	LOG_MSG_INFO
};

class Logger
{
public:
	Logger() noexcept { 
		opened_ = false; 
	}
	Logger(const std::string& log_path, bool erase = false) noexcept { 
		Open(log_path, erase); 
	}
	NPLIB_IMPORT_EXPORT
	~Logger();
	NPLIB_IMPORT_EXPORT
	bool Open(const std::string& log_path, bool erase = false) noexcept;
	NPLIB_IMPORT_EXPORT
	void Close() noexcept;
	NPLIB_IMPORT_EXPORT
	void Write(LOG_MSG_TYPE msg_type, const char *format, ...) noexcept;
private:
	bool opened_;
	std::ofstream ofile_;
	char* GetTime() noexcept;
};

#ifndef NPSYS_EXPORTS
extern Logger logger;
#endif // npsys_EXPORTS

#endif