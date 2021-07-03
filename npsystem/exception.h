#pragma once

class runtime_error_with_ec 
	: public std::runtime_error {
public:
	int code;
	
	runtime_error_with_ec(const char* msg, int _code) 
		: std::runtime_error(msg)
		, code(_code)
	{
	}
};

class operation_stop_algorithm_failed 
	: public std::runtime_error {
public:
	operation_stop_algorithm_failed() 
		: std::runtime_error("operation_stop_algorithm_failed") {}
};

class operation_write_page_failed 
	: public runtime_error_with_ec {
public:
	operation_write_page_failed(int _code)
		: runtime_error_with_ec("operation_write_page_failed", _code) 
	{
	}
};

class operation_replace_algorithm_failed 
	: public runtime_error_with_ec {
public:
	operation_replace_algorithm_failed(int _code)
		: runtime_error_with_ec("operation_replace_algorithm_failed", _code)
	{
	}
};

class compile_error 
	: public runtime_error_with_ec {
public:
	compile_error(int _exit_code)
		: runtime_error_with_ec("compile error", _exit_code)
	{
	}
};

class symbol_not_found 
	: public std::runtime_error {
public:
	symbol_not_found(std::string sym)
		: std::runtime_error("symbol was not found")
		, symbol(sym) {}

	std::string symbol;
};

class function_not_found 
	: public std::runtime_error {
public:
	function_not_found(std::string fn)
		: std::runtime_error("function was not found")
		, func(fn) {}
	
	std::string func;
};

class old_algorithm_was_not_found 
	: public std::runtime_error {
public:
	old_algorithm_was_not_found(uint16_t _addr)
		: std::runtime_error("old_algorithm_was_not_found") 
		, addr(_addr) {}

	uint16_t addr;
};