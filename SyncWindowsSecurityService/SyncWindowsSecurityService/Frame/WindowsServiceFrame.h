#ifndef WindowsServiceFrame_h__
#define WindowsServiceFrame_h__

#include <Windows.h>
#include <tchar.h>

void run_filter();
VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);


#define SERVICE_NAME  _T("Fang Security Service")


#endif // WindowsServiceFrame_h__
