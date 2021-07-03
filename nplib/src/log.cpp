#include <nplib/utils/log.h>
#include <iostream>
#include <ctime>
#include <stdarg.h>

#ifdef _MSC_VER

NPLIB_IMPORT_EXPORT
bool Logger::Open(const std::string& log_path, bool erase) noexcept
{
	ofile_.open(log_path, std::ios::out | (erase ? 0 : std::ofstream::app));
	if (ofile_.fail()) {
		std::cerr << "Couldn't create the log file\n";
		opened_ = false;
	} else {
		opened_ = true;
		Write(LOG_MSG_INFO, "log started");
	}
	return opened_;
}

NPLIB_IMPORT_EXPORT
Logger::~Logger() {
	Close();
}

char* Logger::GetTime() noexcept {
	static char buf[32];
	std::tm tm;
	time_t cur_time = time(nullptr);
	localtime_s(&tm, &cur_time);
	std::strftime(buf, 32, "%a, %d.%m.%Y %H:%M:%S", &tm);
	return buf;
}

NPLIB_IMPORT_EXPORT
void Logger::Write(LOG_MSG_TYPE msg_type, const char *format, ...) noexcept {
	if (!opened_ || ofile_.fail()) return;

	constexpr size_t MAX_SIZE = 1024;
	static char buff[MAX_SIZE];

	va_list args;
	va_start(args, format);
	int size = vsnprintf_s(buff, MAX_SIZE, _TRUNCATE, format, args);
	va_end(args);

	ofile_ << "[" << GetTime() << "] ";

	if (msg_type == LOG_MSG_ERROR) ofile_ << "ERROR: ";
	else if (msg_type == LOG_MSG_INFO) ofile_ << "INFO: ";
	
	ofile_ << buff << std::endl;
}

NPLIB_IMPORT_EXPORT
void Logger::Close() noexcept {
	if (!opened_ || ofile_.fail()) return;

	Write(LOG_MSG_INFO, "log stopped");
	opened_ = false;
	ofile_.close();
}

#endif