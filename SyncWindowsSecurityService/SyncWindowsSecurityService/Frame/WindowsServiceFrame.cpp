#include "WindowsServiceFrame.h"
#include "../Logger/BoostLog.h"
#include "../driver/MinifilterSvr.h"
#include "WindowsServiceThread.h"
#include "RecordCache.h"
#include "../driver/strategy.h"
#include "../Inject/injector.h"
#include "../Encrypted/key/FileKey.h"
#include "../Encrypted/key/Cipher.h"


SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
bool g_InitFlag = false;


using namespace boost;
using namespace std;


VOID ReportSvcStatus( DWORD dwCurrentState,
					 DWORD dwWin32ExitCode,
					 DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.

	g_ServiceStatus.dwCurrentState = dwCurrentState;
	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	g_ServiceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		g_ServiceStatus.dwControlsAccepted = 0;
	else g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ( (dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED) )
		g_ServiceStatus.dwCheckPoint = 0;
	else g_ServiceStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus( g_StatusHandle, &g_ServiceStatus );
}


void CleanupService()
{
    // Perform any cleanup tasks
	SLOG_INFO<<"My Sample Service: ServiceMain: Performing Cleanup Operations";

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    SLOG_INFO<<"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }


	SLOG_INFO<<"My Sample Service: ServiceMain: Exit";
}

#include <tchar.h>
#include <fltuser.h>

void run_filter()
{
	std::shared_ptr<MinifilterSvr> filter(new MinifilterSvr);
	// start security driver
	if (!filter->Init())
	{
		SLOG_ERROR << "init failed";
		//ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		//return;
	}

	Cipher::Instance()->InitCipher();
	FileKey::Instance()->InitKey(Cipher::Instance()->GetFileCipher(), Cipher::Instance()->GetBakFileCiper());

	if (!filter->Listen())
	{
		SLOG_ERROR << "Listen failed";
		//ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		//return;
	}

	SecurityStrategy::Instance()->Filter(filter);
#ifdef SINGLE_TEST
	SecurityStrategy::Instance()->SetEncryptedKey();
	SecurityStrategy::Instance()->SetSecurityPaths(vector<string>{"C:\\Users\\liuyu\\Downloads\\1", "C:\\Users\\liuyu\\Downloads\\2"});
	SecurityStrategy::Instance()->SetWhiteProcessList(vector<SecurityStrategy::WhiteProcess>{
											SecurityStrategy::WhiteProcess{L"notepad++.exe", L"1234567890123456"},
											SecurityStrategy::WhiteProcess{ L"syncwindowssecurityService.exe", L"1234567890123456" },
											SecurityStrategy::WhiteProcess{ L"WINWORD.EXE", L"1234567890123456" } });
	//Injector::Inject(3728, L"C:\\Windows\\System32\\notepad.exe");

#endif
	// pipe server初始化
	WindowsMonitorServiceThread& monitorThread = WindowsMonitorServiceThread::GetInstance();
	monitorThread.Run();

	WindowsNotificationThread& notificationThread = WindowsNotificationThread::GetInstance();
	notificationThread.Run();

	WindowsServiceCtrlThread& serviceCtrlThread = WindowsServiceCtrlThread::GetInstance();
	serviceCtrlThread.Run();
	serviceCtrlThread.Join();

	filter->Stop();
	filter->UnInit();
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;
	SLog::InitLog("security");

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        SLOG_ERROR<<"My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error";
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        SLOG_ERROR<<"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }

    // Perform tasks necessary to start the service here
    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    SLOG_ERROR<<"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }

	g_InitFlag = true;

	run_filter();

	CleanupService();

    return;
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    SLOG_INFO<<"My Sample Service: ServiceCtrlHandler: Entry";

    switch (CtrlCode) 
	{
	case SERVICE_CONTROL_STOP :
		{
			if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
				break;

			ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

			// 用thread自带的interrupt接口
			WindowsServiceCtrlThread& serviceCtrlThread = WindowsServiceCtrlThread::GetInstance();
			serviceCtrlThread.Interrupt();

			ReportSvcStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);

			break;
		}
        
	default:
		{
			break;
		}
		
    }

    SLOG_INFO<<"My Sample Service: ServiceCtrlHandler: Exit";
}

