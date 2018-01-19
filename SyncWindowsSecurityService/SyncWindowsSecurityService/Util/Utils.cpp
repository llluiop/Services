#include "Utils.h"
#include <psapi.h>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <Userenv.h>
#include <ShlObj.h>
#include <memory>

#include "../driver/Strategy.h"
#include "../Util/ScopeGuard.h"

#pragma comment (lib,"Psapi.lib")
#pragma comment (lib,"Userenv.lib")

#define FILTER_DEFAULT_THREAD_COUNT         2

using namespace std;

string Utils::GetExePath()
{
	std::string f = GetExeFileName();
	return f.substr(0, f.find_last_of( "\\/" ));
}


string Utils::GetExeFileName()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA( NULL, buffer, MAX_PATH );

	return std::string(buffer);
}

std::wstring Utils::GetProcessName(DWORD id)
{
#if (_WIN32_WINNT >= 0x0600)  //if vista  
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, id);
#else
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, id);
#endif

	TCHAR procName[MAX_PATH] = { 0 };
	GetModuleFileNameEx(hProcess, NULL, procName, MAX_PATH);

	std::wstring name(procName);
	if (name != L"")
	{
		return name;
	}

	TCHAR procName1[MAX_PATH] = { 0 };
	GetProcessImageFileName(hProcess, procName1, MAX_PATH);
	std::wstring name1(procName1);
	if (name1 != L"")
	{
		return name1;
	}

	TCHAR sExeName[MAX_PATH] = { 0 };
	DWORD size = MAX_PATH;
	QueryFullProcessImageName(hProcess, 0, sExeName, &size);
	std::wstring name2(sExeName);
	if (name2 != L"")
	{
		return name2;
	}

	return std::wstring(procName);
}

std::string Utils::UniqueString(int len)
{
	GUID guid;
	if (S_OK != ::CoCreateGuid(&guid))
		return "";

	unique_ptr<char> buff(new char[len]);
	memcpy(buff.get(), &guid, len);

	return std::string(buff.get(), len);
}

ULONGLONG Utils::GetTickCount()
{
	return GetTickCount64();
}

bool Utils::IsSysProcess(const std::wstring& name)
{
	if (IsSysProcessBlack(name) || IsSysProcessWhite(name))
	{
		return true;
	}

	return false;
}

bool Utils::IsSysProcessBlack(const std::wstring& name)
{
	if (name.find(L"dllhost.exe") != std::wstring::npos)
	{
		return true;
	}

	return false;
}

bool Utils::IsSysProcessWhite(const std::wstring& name)
{
	if (name.find(L"MsMpEng.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"prl_cc.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"SearchProtocolHost.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"SearchFilterHost.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"svchost.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"sihost.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"csrss.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"SearchIndexer.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"baidupinyin.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"OpenWith.exe") != std::wstring::npos)
	{
		return true;
	}

	return false;
}

bool Utils::IsAntivirusProcess(const std::wstring& name)
{
	if (name.find(L"360rp.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"360Safe.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"360sd.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"360Tray.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"QQProtect.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"kxescore.exe") != std::wstring::npos)
	{
		return true;
	}

	if (name.find(L"kxetray.exe") != std::wstring::npos)
	{
		return true;
	}

	return false;
}

bool Utils::IsExplorer(const std::wstring& name)
{
	return name.find(L"explorer.exe") != std::wstring::npos;
}

bool Utils::IsFangCloud(const std::wstring& name)
{
	return name.find(L"FangCloud.exe") != std::wstring::npos;
}

bool Utils::IsSecuritySvr(const std::wstring& name)
{
	return name.find(L"SyncWindowsSecurityService.exe") != std::wstring::npos;
}


bool Utils::IsFolder(const std::wstring & name)
{
	DWORD attr = GetFileAttributes(name.c_str());
	BOOL ret = attr	& FILE_ATTRIBUTE_DIRECTORY;
	return !!ret;
}

bool Utils::IsIM(std::wstring name)
{

	boost::algorithm::to_lower(name);
	if (name.find(L"qq.exe") != wstring::npos)
	{
		return true;
	}
	if (name.find(L"wechat.exe") != wstring::npos)
	{
		return true;
	}
	if (name.find(L"dingtalk.exe") != wstring::npos)
	{
		return true;
	}
	if (name.find(L"foxmail.exe") != wstring::npos)
	{
		return true;
	}

	return false;
}

int Utils::ConcurrencyAbility()
{
	int thread_count = std::thread::hardware_concurrency();
	if (thread_count == 0)
		thread_count = FILTER_DEFAULT_THREAD_COUNT;
	return thread_count * 2;
}

bool Utils::Is32BitExe(const std::wstring exe)
{
	PVOID OldValue = NULL;
	if (Is64BitPlatform())
	{
		Wow64DisableWow64FsRedirection(&OldValue);
	}

	ON_SCOPE_EXIT([=] { 	
		if (Is64BitPlatform())
		{
			Wow64RevertWow64FsRedirection(OldValue);
		}
	});


	IMAGE_DOS_HEADER idh;
	FILE *f = _wfopen(exe.c_str(), _T("rb"));
	if (f == nullptr)
	{
		return false;
	}

	fread(&idh, sizeof(idh), 1, f);

	IMAGE_FILE_HEADER ifh;
	fseek(f, idh.e_lfanew + 4, SEEK_SET);
	fread(&ifh, sizeof(ifh), 1, f);
	fclose(f);



	return ifh.Machine == 0x014C;
}


bool Utils::Is64BitPlatform()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	return (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);
}

bool Utils::CreateProcessWithAdmin(const std::wstring & process_name)
{
	HANDLE hToken = NULL;
	HANDLE hTokenDup = NULL;

	if (process_name.empty()) {
		return false;
	}

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		return false;
	}


	if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityAnonymous, TokenPrimary, &hTokenDup))
	{
		CloseHandle(hToken);
		return false;
	}

	STARTUPINFO si;
	LPVOID pEnv = NULL;
	DWORD dwSessionId = WTSGetActiveConsoleSessionId();

	ZeroMemory(&si, sizeof(STARTUPINFO));

	if (!SetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
	{
		CloseHandle(hToken);
		CloseHandle(hTokenDup);
		return false;
	}

	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = L"WinSta0\\Default";
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION process;
	ZeroMemory(&process, sizeof(process));

	if (!CreateEnvironmentBlock(&pEnv, hTokenDup, FALSE))
	{
		CloseHandle(hToken);
		CloseHandle(hTokenDup);
		return false;
	}

	if (!CreateProcessAsUser(hTokenDup, NULL, (LPWSTR)process_name.c_str(), NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		pEnv, NULL, &si, &process))
	{
		DWORD err = GetLastError();
		CloseHandle(hToken);
		CloseHandle(hTokenDup);
		return false;
	}

	if (pEnv)
	{
		DestroyEnvironmentBlock(pEnv);
	}

	CloseHandle(hToken);
	CloseHandle(hTokenDup);
	return true;
}
