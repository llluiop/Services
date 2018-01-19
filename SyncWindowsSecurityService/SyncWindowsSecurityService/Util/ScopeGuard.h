#ifndef __SCOPEGUARD_H__
#define __SCOPEGUARD_H__


#include <Windows.h>
#include <functional>

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void()> f) :
		f_(f)
	{

	}

	~ScopeGuard()
	{
		f_();
	}

private:
	ScopeGuard(ScopeGuard const&);
	ScopeGuard& operator=(ScopeGuard const&);

private:
	std::function<void()> f_;
};


#endif // 

