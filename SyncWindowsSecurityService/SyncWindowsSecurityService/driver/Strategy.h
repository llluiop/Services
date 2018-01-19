#ifndef	__STRATEGY_H__
#define __STRATEGY_H__

#include "msg.h"
#include "MinifilterSvr.h"
#include "../Encrypted/encrypter.h"
#include "../Logger/BoostLog.h"
#include <filesystem>
#include <ShlObj.h>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>

#define CURRENT_USER _T("liu.txt")
#define FANG _T("FangCloud")


using namespace std::experimental::filesystem::v1;

class SecurityStrategy
{
public:
	static SecurityStrategy* Instance();

	class WhiteProcess
	{
	public:
		std::wstring name_;
		std::wstring md5_;
	};


public:
	bool SetSecurityPaths(const std::vector<std::string>& monitorPaths);
	bool SetEncryptedKey();

	void Filter(const std::shared_ptr<MinifilterSvr>& svr);

	void SetWhiteProcessList(std::vector<WhiteProcess>& whiteProcess)
	{
		std::lock_guard<std::mutex> lck(m_);
		white_process_.swap(whiteProcess);
	}

	bool IsWhiteProcess(const std::wstring& name)
	{
		auto lower_name = name;
		boost::algorithm::to_lower(lower_name);

		std::lock_guard<std::mutex> lck(m_);
		for (auto process : white_process_)
		{
			if (lower_name.find(process.name_) != wstring::npos)
			{
				return true;
			}
		}

		return false;
	}

private:
	std::shared_ptr<MinifilterSvr> svr_;
	std::vector<WhiteProcess> white_process_;
	std::mutex m_;

private:
	SecurityStrategy() {};
	SecurityStrategy(const SecurityStrategy&) = delete;
	SecurityStrategy& operator=(const SecurityStrategy&) = delete;

};

#endif

