#ifndef RecordCache_h__
#define RecordCache_h__


#include <Windows.h>
#include <WinIoCtl.h>
#include <string>
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>


class RecordCache
{
public:
	RecordCache() {}
	~RecordCache() { cache_.clear(); }
	void Insert(DWORDLONG fileReferenceNumber, int type, boost::shared_ptr<USN_RECORD> recordPtr);

public:
	std::map<DWORDLONG, std::map<int, boost::shared_ptr<USN_RECORD>>> cache_;

};


// ��Ϊһ����ʵ�������cache�������ܴ��ڶ���߳�ͬʱ���ʣ���ô��д������ʱ��һ��Ҫ����������������ִ���

class RecordCacheManager : boost::noncopyable
{
public:
	static RecordCacheManager& GetInstance();
	static void DestroyInstance();

	void Insert(boost::shared_ptr<USN_RECORD> record);
	bool Ready(DWORDLONG fileReferenceNumber);
	void Post(DWORDLONG fileReferenceNumber, std::vector<boost::shared_ptr<USN_RECORD>>& output);

private:
	RecordCacheManager();

public:
	static RecordCacheManager *instance_;

private:
	boost::mutex io_mutex;
	RecordCache cache_;
};


#endif // RecordCache_h__
