// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifdef _WIN32

#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <nplib/utils/win_service.h>
#include "service_error.h"


#pragma comment(lib, "advapi32.lib")

LPCTSTR Service::service_name_;
SERVICE_STATUS Service::gSvcStatus;
SERVICE_STATUS_HANDLE Service::gSvcStatusHandle;
std::function<int(int, char**, Service::ready_t*)> Service::main_;
std::function<void()> Service::stop_;

/*
Routine Description:
		This routine returns if the service specified is not running interactively
		(not invoked \by the service controller).
*/


NPLIB_IMPORT_EXPORT void Service::ready_t::operator()() {
	Service::ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
}

NPLIB_IMPORT_EXPORT
bool Service::IsServiceMode() noexcept {

	HANDLE hProcessToken = NULL;
	DWORD groupLength = 50;

	PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0,
		groupLength);

	SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
	PSID InteractiveSid = NULL;
	PSID ServiceSid = NULL;
	DWORD i;

	// Start with assumption that process is an EXE, not a Service.
	bool fExe = true;


	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,
		&hProcessToken))
		goto ret;

	if (groupInfo == NULL)
		goto ret;

	if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
		groupLength, &groupLength))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto ret;

		LocalFree(groupInfo);
		groupInfo = NULL;

		groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

		if (groupInfo == NULL)
			goto ret;

		if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
			groupLength, &groupLength))
		{
			goto ret;
		}
	}

	//
	//  We now know the groups associated with this token.  We want to look to see if
	//  the interactive group is active in the token, and if so, we know that
	//  this is an interactive process.
	//
	//  We also look for the "service" SID, and if it's present, we know we're a service.
	//
	//  The service SID will be present iff the service is running in a
	//  user account (and was invoked by the service controller).
	//


	if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0,
		0,
		0, 0, 0, 0, 0, &InteractiveSid))
	{
		goto ret;
	}

	if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0,
		0, 0, 0, 0, &ServiceSid))
	{
		goto ret;
	}

	for (i = 0; i < groupInfo->GroupCount; i += 1)
	{
		SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
		PSID Sid = sanda.Sid;

		//
		//  Check to see if the group we're looking at is one of
		//  the 2 groups we're interested in.
		//

		if (EqualSid(Sid, InteractiveSid))
		{
			//
			//  This process has the Interactive SID in its
			//  token.  This means that the process is running as
			//  an EXE.
			//
			goto ret;
		} else if (EqualSid(Sid, ServiceSid))
		{
			//
			//  This process has the Service SID in its
			//  token.  This means that the process is running as
			//  a service running in a user account.
			//
			fExe = false;
			goto ret;
		}
	}

	//
	//  Neither Interactive or Service was present in the current users token,
	//  This implies that the process is running as a service, most likely
	//  running as LocalSystem.
	//
	fExe = false;

ret:

	if (InteractiveSid)
		FreeSid(InteractiveSid);

	if (ServiceSid)
		FreeSid(ServiceSid);

	if (groupInfo)
		LocalFree(groupInfo);

	if (hProcessToken)
		CloseHandle(hProcessToken);

	return !fExe;
}

NPLIB_IMPORT_EXPORT
Service::Service(LPCTSTR service_name) {
	service_name_ = service_name;
}

NPLIB_IMPORT_EXPORT
void Service::Start(
	std::function<int(int, char**, ready_t*)> main,
	std::function<void()> stop
) {
	main_ = main;
	stop_ = stop;
	// TO_DO: Add any additional services for the process to this table.
	static SERVICE_TABLE_ENTRY DispatchTable[] =
	{
			{ (LPTSTR)service_name_, (LPSERVICE_MAIN_FUNCTION)SvcMain },
			{ NULL, NULL }
	};
	// This call returns when the service has stopped. 
	// The process should simply terminate when the call returns.
	if (!StartServiceCtrlDispatcher(DispatchTable)) {
		SvcReportEvent((LPTSTR)L"StartServiceCtrlDispatcher");
	}
}

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
NPLIB_IMPORT_EXPORT
VOID Service::SvcInstall(LPCTSTR dependencies) {
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Create the service

	schService = CreateService(
		schSCManager,              // SCM database 
		service_name_,             // name of service 
		service_name_,             // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		dependencies,              // dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL) {
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	} else printf("Service installed successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI Service::SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// Register the handler function for the service

	gSvcStatusHandle = RegisterServiceCtrlHandler(service_name_, SvcCtrlHandler);

	if (!gSvcStatusHandle)
	{
		SvcReportEvent((LPTSTR)TEXT("RegisterServiceCtrlHandler"));
		return;
	}

	// These SERVICE_STATUS members remain as set here

	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;

	// Report initial status to the SCM

	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Perform service-specific initialization and work.

	SvcInit(dwArgc, lpszArgv);
}


//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//

VOID Service::SvcInit(DWORD dwArgc, LPTSTR *lpszArgv) {
	 //std::ofstream out("d:\\temp\\" + std::string(service_name_) + ".log");
   //std::cout.rdbuf(out.rdbuf());
   //std::cerr.rdbuf(out.rdbuf());

	// TO_DO: Declare and set any required variables.
	//   Be sure to periodically call ReportSvcStatus() with 
	//   SERVICE_START_PENDING. If initialization fails, call
	//   ReportSvcStatus with SERVICE_STOPPED.

	// Report running status when initialization is complete.

	auto argv = std::make_unique<char*[]>(dwArgc);
	auto strs = std::make_unique<std::string[]>(dwArgc);

	for (DWORD i = 0; i < dwArgc; ++i) {
		strs[i] = lpszArgv[i];
		argv[i] = strs[i].data();
	}

	// Perform work until service stops.
	ready_t ready;
	auto ok = main_(dwArgc, argv.get(), &ready);
	ReportSvcStatus(SERVICE_STOPPED, ok, 0);
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI Service::SvcCtrlHandler(DWORD dwCtrl) {
	// Handle the requested control code. 
	switch (dwCtrl)
	{
	// Signal the service to stop.
	case SERVICE_CONTROL_STOP:
		ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		stop_();
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	}
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//

VOID Service::ReportSvcStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}



//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID Service::SvcReportEvent(LPTSTR szFunction)
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	TCHAR Buffer[80];

	hEventSource = RegisterEventSource(NULL, service_name_);

	if (NULL != hEventSource)
	{
		StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

		lpszStrings[0] = service_name_;
		lpszStrings[1] = Buffer;

		ReportEvent(hEventSource,        // event log handle
			EVENTLOG_ERROR_TYPE, // event type
			0,                   // event category
			SVC_ERROR,           // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}

#endif // WIN32

/*
BOOL __stdcall StopDependentServices()
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;
    ENUM_SERVICE_STATUS     ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeout = 30000; // 30-second time-out

    // Pass a zero-length buffer to get the required buffer size.
    if ( EnumDependentServices( schService, SERVICE_ACTIVE, 
         lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) 
    {
         // If the Enum call succeeds, then there are no dependent
         // services, so do nothing.
         return TRUE;
    } 
    else 
    {
        if ( GetLastError() != ERROR_MORE_DATA )
            return FALSE; // Unexpected error

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
            GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );
  
        if ( !lpDependencies )
            return FALSE;

        __try {
            // Enumerate the dependencies.
            if ( !EnumDependentServices( schService, SERVICE_ACTIVE, 
                lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                &dwCount ) )
            return FALSE;

            for ( i = 0; i < dwCount; i++ ) 
            {
                ess = *(lpDependencies + i);
                // Open the service.
                hDepService = OpenService( schSCManager, 
                   ess.lpServiceName, 
                   SERVICE_STOP | SERVICE_QUERY_STATUS );

                if ( !hDepService )
                   return FALSE;

                __try {
                    // Send a stop code.
                    if ( !ControlService( hDepService, 
                            SERVICE_CONTROL_STOP,
                            (LPSERVICE_STATUS) &ssp ) )
                    return FALSE;

                    // Wait for the service to stop.
                    while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
                    {
                        Sleep( ssp.dwWaitHint );
                        if ( !QueryServiceStatusEx( 
                                hDepService, 
                                SC_STATUS_PROCESS_INFO,
                                (LPBYTE)&ssp, 
                                sizeof(SERVICE_STATUS_PROCESS),
                                &dwBytesNeeded ) )
                        return FALSE;

                        if ( ssp.dwCurrentState == SERVICE_STOPPED )
                            break;

                        if ( GetTickCount() - dwStartTime > dwTimeout )
                            return FALSE;
                    }
                } 
                __finally 
                {
                    // Always release the service handle.
                    CloseServiceHandle( hDepService );
                }
            }
        } 
        __finally 
        {
            // Always free the enumeration buffer.
            HeapFree( GetProcessHeap(), 0, lpDependencies );
        }
    } 
    return TRUE;
}
*/