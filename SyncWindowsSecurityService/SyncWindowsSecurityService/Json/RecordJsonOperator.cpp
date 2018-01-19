#include "RecordJsonOperator.h"
//#include <json/json.h>
#include "RequestDefine.h"
#include "../Util/StrConvert.h"
#include "../Logger/BoostLog.h"


using namespace std;


bool RecordJsonOperator::Record2JsonStr(const Record &record, std::string &jsonStr)
{
	//Json::Value root;
	//root["path"] = Json::Value(record.Path());
	//root["reason"] = Json::Value(record.Reason());
	//root["action"] = Json::Value(record.Action());

	//std::string strLastUSN = StrConvert::FromLongLongToStr(record.LastUSN());
	//root["lastUSN"] = Json::Value(strLastUSN);

	//jsonStr = root.toStyledString();
	return true;
}


bool RecordJsonOperator::JsonStr2Record(const std::string &jsonStr, Record &record)
{
	//Json::Reader reader;    
	//Json::Value root;    
	//if (reader.parse(jsonStr, root))     
	//{    
	//	std::string path = root["path"].asString(); 
	//	record.SetPath(path);

	//	std::string reason = root["reason"].asString(); 
	//	record.SetReason(reason);

	//	int action = root["action"].asInt();
	//	record.SetAction(action);

	//	std::string strLastUSN = root["lastUSN"].asString();
	//	LONGLONG lastUSN = StrConvert::FromStrToLongLong(strLastUSN);

	//	record.SetLastUSN(lastUSN);
	//	return true;
	//}    
	//else
	//{
	//	return false;
	//}
	return true;
}


bool RecordJsonOperator::Records2JsonStr(const Records &records, std::string &jsonStr)
{
	// records这里可能包含重复的记录
	// 例如，两个相邻的usn记录是针对一个文件，而且action的类型也是一样的
	// 其实可以做一个过滤，出现这种事情的时候，只保留一个记录（文件名称、type）
	// 这样可以提高效率
	//Records recordsAfterFilter;
	//RecordsFilter(records, recordsAfterFilter);


	//Json::Value root;
	//const std::vector<Record> &allRecords = recordsAfterFilter.GetRecords();
	////const std::vector<Record> &allRecords = records.GetRecords();
	//std::vector<Record>::const_iterator cur = allRecords.begin();
	//Json::Value arrayObj;
	//while (cur != allRecords.end())
	//{
	//	// 但是这个里面是GB2312编码的，直接发给sync不好
	//	// 所以在这里改成utf-8格式的，主要针对string格式变量
	//	Json::Value item;
	//	std::string path = StrConvert::FromGb2312ToUtf8((*cur).Path()) ;
	//	item["path"] = Json::Value(path);
	//	item["reason"] = Json::Value((*cur).Reason());
	//	item["action"] = Json::Value((*cur).Action());
	//	// last usn number的范围是LONGLONG类型的，一般的int类型handle不住
	//	// 这里可以利用str类型存储last usn number
	//	// 在python端也是用str接收，存储，发送
	//	// 唯一还需要改变的是在接收端的地方需要将str转换成LONGLONG
	//	// from long long to str
	//	std::string strLastUSN = StrConvert::FromLongLongToStr((*cur).LastUSN());
	//	item["lastUSN"] = Json::Value(strLastUSN);

	//	arrayObj.append(item);

	//	++cur;
	//}

	//root["records"] = arrayObj;
	//root["type"] = Json::Value(kRecordsRequest);
	//jsonStr = root.toStyledString();

	return true;
}


void RecordJsonOperator::RecordsFilter(const Records &records, Records &recordsAfterFilter)
{
	// 
	unsigned int size = records.Size();
	if (size <= 0)
	{
		return;
	}

	const std::vector<Record> &allRecords = records.GetRecords();
	unsigned int index = 0;
	while (index < size)
	{
		unsigned int indexPost = index + 1;
		if (indexPost < size)
		{
			if (allRecords[index].Action() == allRecords[indexPost].Action() && allRecords[index].Path() == allRecords[indexPost].Path())
			{
				// do nothing
			}
			else
			{
				recordsAfterFilter.Insert(allRecords[index]);
			}
		}
		else
		{
			recordsAfterFilter.Insert(allRecords[index]);
		}

		++index;
	}


}


bool RecordJsonOperator::JsonStr2Records(const std::string &jsonStr, Records &records)
{
	//Json::Reader reader;    
	//Json::Value root;    
	//if (reader.parse(jsonStr, root))     
	//{
	//	Json::Value arrayObj = root["records"];
	//	unsigned int size = arrayObj.size();
	//	for (unsigned int i = 0; i < size; i++)
	//	{
	//		Record record;
	//		if (arrayObj[i].isMember("path"))
	//		{
	//			record.SetPath(arrayObj[i]["path"].asString());
	//		}
	//		if (arrayObj[i].isMember("reason"))
	//		{
	//			record.SetReason(arrayObj[i]["reason"].asString());
	//		}
	//		if (arrayObj[i].isMember("action"))
	//		{
	//			record.SetAction(arrayObj[i]["action"].asInt());
	//		}
	//		if (arrayObj[i].isMember("lastUSN"))
	//		{
	//			std::string strLastUSN = arrayObj[i]["lastUSN"].asString();
	//			LONGLONG lastUSN = StrConvert::FromStrToLongLong(strLastUSN);

	//			record.SetLastUSN(lastUSN);
	//		}
	//		records.Insert(record);
	//	}

	//	return true;
	//}    
	//else
	//{
	//	return false;
	//}
	return true;
}
