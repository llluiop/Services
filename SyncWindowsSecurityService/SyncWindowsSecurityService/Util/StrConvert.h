#ifndef StrConvert_h__
#define StrConvert_h__


#include <Windows.h>
#include <string>


class StrConvert{
public:
	static std::string FromUtf8ToGb2312(const std::string& input);
	static std::string FromGb2312ToUtf8(const std::string& input);

	static std::string FromLongLongToStr(const LONGLONG input);
	static LONGLONG FromStrToLongLong(const std::string& input);
};


#endif // StrConvert_h__
