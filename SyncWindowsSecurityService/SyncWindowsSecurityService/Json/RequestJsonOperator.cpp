#include "RequestJsonOperator.h"
#include "../Util/StrConvert.h"
#include "../Logger/BoostLog.h"
#include "../driver/strategy.h"
#include "../storage/storage.h"
#include <json/json.h>
using namespace std;


int RequestJsonOperator::GetRequestType(const std::string& request)
{
	int type = kErrorRequest;
	Json::Reader reader;    
	Json::Value root;    
	if (reader.parse(request, root))     
	{    
		type = root["type"].asInt();
	}    
	
	SLOG_INFO << "RequestJsonOperator get request type" << type;
	return type;
}


bool RequestJsonOperator::UpdatePaths(const std::string& request)
{
	std::vector<std::string> monitorPaths;
	
	Json::Reader reader;    
	Json::Value root;    
	if (reader.parse(request, root))     
	{    
		Json::Value arrayObj = root["security_paths"];
		unsigned int size = arrayObj.size();
		
		for(unsigned int i = 0; i < size; i++)
		{
			std::string monitorPath = arrayObj[i].asString();
			monitorPaths.push_back(monitorPath);
		}
	}    

	if (!SecurityStrategy::Instance()->SetSecurityPaths(monitorPaths))
	{
		SLOG_INFO << "SetSecurityPaths failed";
		return false;
	}

	return Storage::SavePath(monitorPaths);
}

bool RequestJsonOperator::UpdateWhiteProcessList(const std::string& request)
{
	std::vector<SecurityStrategy::WhiteProcess> processes;

	Json::Reader reader;
	Json::Value root;
	if (reader.parse(request, root))
	{
		Json::Value arrayObj = root["white_processes"];
		unsigned int size = arrayObj.size();

		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		for (unsigned int i = 0; i < size; i++)
		{
			SecurityStrategy::WhiteProcess process;
			process.name_ = conv.from_bytes(arrayObj[i]["process_name"].asString());
			process.md5_ = conv.from_bytes(arrayObj[i]["md5"].asString());

			boost::algorithm::to_lower(process.name_);
			processes.push_back(process);

			SLOG_INFO << "WHITE_IMAGE_INFO:" << process.name_ << process.md5_;
		}
	}

	SecurityStrategy::Instance()->SetWhiteProcessList(processes);

	return Storage::SaveWhiteProcessList(processes);

}
