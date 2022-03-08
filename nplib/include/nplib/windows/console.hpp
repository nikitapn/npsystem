#pragma once

#include <windows.h>
#include <string_view>

namespace nplib::win {
inline bool create_console(std::string_view title, int width, int height) {
	if (!AllocConsole()) return false;
	
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);

	SetConsoleTitleA(title.data());
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	auto hConsole = GetConsoleWindow();
	
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	MoveWindow(
		hConsole, 
		screenWidth - width, 
		screenHeight - height - 20, 
		width, height, 
		FALSE);
	
	SetWindowPos(hConsole, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	std::cout.clear();
	std::cerr.clear();
	std::clog.clear();
	std::cin.clear();

	return true;
}
}; // namespace nplib::win