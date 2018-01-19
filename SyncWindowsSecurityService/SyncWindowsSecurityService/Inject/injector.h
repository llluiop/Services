#ifndef __INJECT_H__
#define __INJECT_H__

#include <Windows.h>
#include <string>
#include <unordered_set>
#include <cmath>
#include <algorithm>

class Injector
{
public:
	struct Record
	{
		DWORD id;
		std::wstring name;
	};

	struct Hash
	{
		size_t operator()(const Record& rhs) const {
			return std::hash<std::wstring>()(rhs.name) ^ std::hash<int>()(rhs.id);
		}
	};
	struct Cmp
	{
		bool operator()(const Record& lhs, const Record& rhs) const {
			return lhs.name == rhs.name && lhs.id == rhs.id;
		}
	};


	static bool Inject(DWORD process_id, const std::wstring& exe);
	static bool Launch(DWORD process_id, const std::wstring& exe, bool is32bit);

private:
	static std::wstring InjectorPath(bool is32bit);

private:
	static std::unordered_set<Record, Hash, Cmp> injected_;
};

#endif
