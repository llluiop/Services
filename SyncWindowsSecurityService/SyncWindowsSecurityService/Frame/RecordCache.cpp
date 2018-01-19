#include "RecordCache.h"
#include "MonitorDefine.h"


using namespace std;


void RecordCache::Insert(DWORDLONG fileReferenceNumber, int type, boost::shared_ptr<USN_RECORD> recordPtr)
{
	std::map<DWORDLONG, std::map<int, boost::shared_ptr<USN_RECORD>>>::iterator cur = cache_.find(fileReferenceNumber);
	if (cache_.end() != cur)
	{
		cur->second[type] = recordPtr;
	}
	else
	{
		std::map<int, boost::shared_ptr<USN_RECORD>> value;
		value[type] = recordPtr;
		cache_.insert(make_pair(fileReferenceNumber, value));
	}
	
}


RecordCacheManager *RecordCacheManager::instance_ = NULL;


RecordCacheManager& RecordCacheManager::GetInstance()
{
	if (NULL == instance_)
		instance_ = new RecordCacheManager;

	return *instance_;
}


void RecordCacheManager::DestroyInstance()
{
	if (NULL != instance_)
	{
		delete instance_;
		instance_ = NULL;
	}
}


RecordCacheManager::RecordCacheManager()
{

}


void RecordCacheManager::Insert(boost::shared_ptr<USN_RECORD> recordPtr)
{
	boost::mutex::scoped_lock lock(io_mutex);

	DWORDLONG fileReferenceNumber = recordPtr->FileReferenceNumber;
	if (0 != (recordPtr->Reason & USN_REASON_RENAME_OLD_NAME))
	{
		cache_.Insert(fileReferenceNumber, 0, recordPtr);
	}
	else if (0 != (recordPtr->Reason & USN_REASON_RENAME_NEW_NAME))
	{
		cache_.Insert(fileReferenceNumber, 1, recordPtr);
	}
	else
	{
		// do nothing
	}

}


bool RecordCacheManager::Ready(DWORDLONG fileReferenceNumber)
{
	if (cache_.cache_.find(fileReferenceNumber) != cache_.cache_.end())
	{
		if (cache_.cache_[fileReferenceNumber].find(0) != cache_.cache_[fileReferenceNumber].end() && 
			cache_.cache_[fileReferenceNumber].find(1) != cache_.cache_[fileReferenceNumber].end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


void RecordCacheManager::Post(DWORDLONG fileReferenceNumber, std::vector<boost::shared_ptr<USN_RECORD>>& output)
{
	boost::mutex::scoped_lock lock(io_mutex);

	std::map<DWORDLONG, std::map<int, boost::shared_ptr<USN_RECORD>>>::iterator find = cache_.cache_.find(fileReferenceNumber);
	if (cache_.cache_.end() != find)
	{
		boost::shared_ptr<USN_RECORD> recordPre = cache_.cache_[fileReferenceNumber][0];
		boost::shared_ptr<USN_RECORD> recordLater = cache_.cache_[fileReferenceNumber][1];

		output.push_back(recordPre);
		output.push_back(recordLater);

		cache_.cache_.erase(find);
	}
	
}
