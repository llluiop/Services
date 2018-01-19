#ifndef __utils_h__
#define __utils_h__


#include <Windows.h>
#include <string>


class Utils{
public:
	static std::string GetExePath();

	static std::string GetExeFileName();

	static std::wstring GetProcessName(DWORD id);

	static std::string UniqueString(int len);

	static ULONGLONG GetTickCount();

	static bool IsSysProcess(const std::wstring& name);

	static bool IsSysProcessBlack(const std::wstring & name);

	static bool IsSysProcessWhite(const std::wstring & name);

	static bool IsAntivirusProcess(const std::wstring& name);

	static bool IsExplorer(const std::wstring& name);

	static bool IsFangCloud(const std::wstring& name);

	static bool IsSecuritySvr(const std::wstring& name);

	static bool IsFolder(const std::wstring& name);

	static bool IsIM(std::wstring name);

	static int ConcurrencyAbility();

	static bool Is32BitExe(const std::wstring exe);

	static bool Is64BitPlatform();

	static bool CreateProcessWithAdmin(const std::wstring & process_name);
};


#endif // __utils_h__
