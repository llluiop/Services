#include <Windows.h>
#include <tchar.h>
// 在debug模式下试试vld
//#ifdef _DEBUG
//#include <vld.h>
//#endif

#include "WindowsServiceFrame.h"

int _tmain (int argc, TCHAR *argv[])
{
    OutputDebugString(_T("My Sample Service: Main: Entry"));
#ifdef SINGLE_TEST
	run_filter();
#endif
    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       OutputDebugString(_T("My Sample Service: Main: StartServiceCtrlDispatcher returned error"));
       return GetLastError ();
    }
    OutputDebugString(_T("My Sample Service: Main: Exit"));
    return 0;
}
