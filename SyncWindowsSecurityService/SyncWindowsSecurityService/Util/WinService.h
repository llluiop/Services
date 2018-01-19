#ifndef __WINSERVICE_H__
#define __WINSERVICE_H__

#include <Windows.h>
#include <string>
#include <tchar.h>
#include "../Logger/BoostLog.h"
#include "ScopeGuard.h"

using namespace std;

class WinService
{
public:
	static bool StartWinService(const wstring& name)
	{
		SC_HANDLE manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
		if (manager == nullptr)
		{
			SLOG_ERROR << boost::format("OpenSCManager Failed: %1%") % GetLastError();
			return false;
		}
		ON_SCOPE_EXIT([=] {CloseServiceHandle(manager); });

		SC_HANDLE service = OpenService(manager, name.c_str(), SERVICE_START);
		if (service == nullptr)
		{
			SLOG_ERROR << boost::format("OpenService Failed: %1%") % GetLastError();
			return false;
		}
		ON_SCOPE_EXIT([=] {CloseServiceHandle(service); });

		if (!StartService(service, 0, nullptr))
		{
			SLOG_ERROR << boost::format("StartService Failed: %1%") % GetLastError();
			return false;
		}

		return true;
	}

	static bool StopWinService(const wstring& name)
	{
		SC_HANDLE manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
		if (manager == nullptr)
		{
			SLOG_ERROR << boost::format("OpenSCManager Failed: %1%") % GetLastError();
			return false;
		}
		ON_SCOPE_EXIT([&] {CloseServiceHandle(manager); });

		SC_HANDLE service = OpenService(manager, name.c_str(), SERVICE_STOP);
		if (service == nullptr)
		{
			SLOG_ERROR << boost::format("OpenService Failed: %1%") % GetLastError();
			return false;
		}
		ON_SCOPE_EXIT([&] {CloseServiceHandle(service); });

		SERVICE_STATUS status;
		if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
		{
			SLOG_ERROR << boost::format("ControlService Failed: %1% %2% %3%") % GetLastError()
				% status.dwCurrentState % status.dwWin32ExitCode;
			return false;
		}

		return true;
	}
};

#endif