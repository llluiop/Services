#include "injector.h"
#include "../Util/Utils.h"
#include <tchar.h>

#define INJECTOR "dll_installer.exe"

#define PATH_32(x) L"x32\\"##x
#define PATH_64(x) L"x64\\"##x

#define INJECTOR_32 PATH_32(INJECTOR) 
#define INJECTOR_64 PATH_64(INJECTOR)

std::unordered_set<Injector::Record, Injector::Hash, Injector::Cmp> Injector::injected_;

bool Injector::Inject(DWORD process_id, const std::wstring& exe)
{
	if (injected_.find({ process_id, exe}) != injected_.end())
	{
		return true;
	}

	auto is_32_bit = Utils::Is32BitExe(exe);
	if (Launch(process_id, exe, is_32_bit))
	{
		injected_.insert({ process_id, exe });
		return true;
	}

	return false;
}

bool Injector::Launch(DWORD process_id, const std::wstring& exe, bool is32bit)
{
	std::wstring cmd_param = std::to_wstring(process_id);
	cmd_param.append(L" ");
	cmd_param.append(exe);

	auto path = InjectorPath(is32bit);

	path.append(L" ");
	path.append(cmd_param);

	return Utils::CreateProcessWithAdmin(path);
}

std::wstring Injector::InjectorPath(bool is32bit)
{
	TCHAR cWinDir[MAX_PATH] = {0};
	GetModuleFileName(nullptr, cWinDir,MAX_PATH );
	(_tcsrchr(cWinDir, ('\\')))[1] = 0;

	std::wstring path(cWinDir);
	if (is32bit)
	{
		path.append(INJECTOR_32);
	}
	else
	{
		path.append(INJECTOR_64);
	}

	return path;
}
