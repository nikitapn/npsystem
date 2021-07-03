#pragma once

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <functional>
#include "../import_export.h"
#include <condition_variable>
#include <mutex>

class Service {
	friend struct ready_t;

public:
	struct ready_t {
		NPLIB_IMPORT_EXPORT void operator()();
	};

	NPLIB_IMPORT_EXPORT
		static bool IsServiceMode() noexcept;
	NPLIB_IMPORT_EXPORT
		Service(LPCTSTR service_name);
	NPLIB_IMPORT_EXPORT
		void Start(
			std::function<int(int, char**, ready_t*)> main,
			std::function<void()> stop
		);
	NPLIB_IMPORT_EXPORT
		VOID SvcInstall(LPCTSTR dependencies = TEXT("\0\0"));
private:
	static LPCTSTR service_name_;
	static SERVICE_STATUS gSvcStatus;
	static SERVICE_STATUS_HANDLE gSvcStatusHandle;

	static VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv);
	static VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv);
	static VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
	static VOID ReportSvcStatus(DWORD, DWORD, DWORD);
	static VOID SvcReportEvent(LPTSTR);

	static std::function<int(int, char**, ready_t*)> main_;
	static std::function<void()> stop_;
};