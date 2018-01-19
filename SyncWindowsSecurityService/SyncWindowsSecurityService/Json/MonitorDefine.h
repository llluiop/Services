#ifndef MonitorDefine_h__
#define MonitorDefine_h__


#include <Windows.h>
#include <string>
#include <map>
#include <vector>


#define USN_REASON_CUT     (0x01000000)


// 只支持volume name大写的方式
class MonitorInfo{
public:
	void SetMonitorInfo(const std::map<char, std::vector<std::string>>& monitorInfo){monitorInfo_ = monitorInfo;}
	const std::map<char, std::vector<std::string>>& GetMonitorInfo() const {return monitorInfo_;}

private:
	std::map<char, std::vector<std::string>> monitorInfo_;

};


class LastUSNInfo{
public:
	void SetLastUSNInfo(const std::map<char, LONGLONG>& lastUSN);
	const std::map<char, LONGLONG>& GetLastUSNInfo() const {return lastUSN_;}	

	LONGLONG GetSelectUSNNumber(const char volumeName);

private:
	std::map<char, LONGLONG> lastUSN_;

};


inline void LastUSNInfo::SetLastUSNInfo(const std::map<char, LONGLONG>& lastUSN)
{
	std::map<char, LONGLONG>::const_iterator cur = lastUSN.begin();
	while (lastUSN.end() != cur)
	{
		char key = toupper((*cur).first);
		LONGLONG value = (*cur).second;


		lastUSN_[key] = value;
		++cur;
	}
	
}


inline LONGLONG LastUSNInfo::GetSelectUSNNumber(const char volumeName)
{
	// 此处应该支持volumeName的大小写
	// 无论volumeName是大写还是小写，都转换成大写
	char volumeNameUpper = toupper(volumeName);

	std::map<char, LONGLONG>::iterator find = lastUSN_.find(volumeNameUpper);
	if (find != lastUSN_.end())
	{
		// 找到满足要求的
		return (*find).second;
	}
	else
	{
		return 0;
	}

}


#endif // MonitorDefine_h__
