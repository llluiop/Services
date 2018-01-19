#include "StrConvert.h"
#include <Windows.h>
#include <WinNls.h>


using namespace std;


std::string StrConvert::FromUtf8ToGb2312(const std::string& input)
{
	//
	int len = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return std::string(str);
}


std::string StrConvert::FromGb2312ToUtf8(const std::string& input)
{
	// 
	int len = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return std::string(str);
}


std::string StrConvert::FromLongLongToStr(const LONGLONG input)
{
	// 后面是否需要增加安全检验
	// 如果函数执行失败，需要程序正常返回
	char buf[65] = {0};
	_i64toa_s(input, buf, sizeof(buf), 10); //返回"12345678"
	std::string strJournalId = buf;

	return strJournalId;
}


LONGLONG StrConvert::FromStrToLongLong(const std::string& input)
{
	LONGLONG journalId = _atoi64(input.c_str());
	return journalId;
}