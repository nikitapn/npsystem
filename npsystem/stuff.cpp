#include "stdafx.h"
#include "stuff.h"

int CreateWindowProccessA(const std::string& stCmd, const std::string& path) {
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if (CreateProcessA(NULL, const_cast<char*>(stCmd.c_str()), NULL, NULL, TRUE, NULL, NULL,
		path.empty() ? NULL : path.c_str(), &si, &pi) == FALSE) {
		std::cerr <<  "Error in CreateProccess" << std::endl;
		return -1;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	return exit_code;
}

int CreateProccessWindowlessA(const std::string& stCmd, const std::string& path) {
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if (CreateProcessA(NULL, (char*)stCmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, 
		path.empty() ? NULL : path.c_str(), &si, &pi) == FALSE) {
		std::cerr << "Error in CreateProccess" << std::endl;
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	return exit_code;
}

int CreateProccessWindowlessA(const std::string& cmd, const std::string& path, std::ostream& out) {
	HANDLE hStdWrite, hStdRead;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (CreatePipe(&hStdRead, &hStdWrite, &sa, 0) == FALSE) {
		std::cerr << "Error in CreatePipe" << std::endl;
		return -1;
	}

	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	si.hStdError = hStdWrite;
	si.hStdOutput = hStdWrite;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	
	{
		/*
		lpCommandLine [in, out, optional]
			The Unicode version of this function, CreateProcessW, can modify the contents of this string. 
			Therefore, this parameter cannot be a pointer to read-only memory (such as a const variable or a literal string). 
			If this parameter is a constant string, the function may cause an access violation.
		*/
		auto const cmd_len = cmd.length() + 1;
		auto cmd_line = std::make_unique<char[]>(cmd_len);
		strcpy_s(cmd_line.get(), cmd_len, cmd.c_str());

		if (CreateProcessA(NULL, cmd_line.get(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL
			, path.empty() ? NULL : path.c_str()
			, &si, &pi) == FALSE) {
			CloseHandle(hStdWrite);
			CloseHandle(hStdRead);
			std::cerr << "Error in CreateProccess: "
				<< std::hex << std::setw(8) << std::setfill('0') << GetLastError() << std::endl;
			return -1;
		}
	}

	DWORD avail, cbRead, dwProcState = -1;

	Sleep(200);

	constexpr size_t buf_max_len = 1024;
	static char buf[buf_max_len];
	bool stop = false;

	for(;;) {
		if (PeekNamedPipe(hStdRead, NULL, NULL, NULL, &avail, NULL) == FALSE) {
			std::cerr << "Error in PeekNamedPipe: " << std::hex << GetLastError() << std::endl;
		}

		if (avail) {
			DWORD readed = 0;
			while (readed < avail) {
				ReadFile(hStdRead, buf, buf_max_len, &cbRead, NULL);
				readed += cbRead;
				out << std::string_view(buf, cbRead);
			}
		}

		MSG uMsg;
		if (PeekMessage(&uMsg, g_hMainWnd, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&uMsg);
			::DispatchMessage(&uMsg);
		}
		
		if (stop) break;
		
		GetExitCodeProcess(pi.hProcess, &dwProcState);
		
		if (dwProcState != STILL_ACTIVE) {
			Sleep(250);
			stop = true;
		}
	}

	CloseHandle(hStdWrite);
	CloseHandle(hStdRead);
	CloseHandle(pi.hProcess);

	return dwProcState;
}