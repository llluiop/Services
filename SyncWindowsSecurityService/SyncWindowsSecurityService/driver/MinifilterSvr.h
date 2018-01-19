#ifndef __MINIFILTER_H__
#define __MINIFILTER_H__
#include <Windows.h>
#include <vector>
#include "../Util/HeartBeat.h"
#include "../Util/Utils.h"
#include "msg.h"

#define FILTER_TIME_OUT                     1500

class MinifilterSvr
{
public:
	MinifilterSvr();
	~MinifilterSvr();

public:
	bool Init();
	bool UnInit();

	bool Listen();
	void Stop();

	bool SendHeartBeat(const LPVOID msg, const DWORD len) const;
	bool SendSecurityPath(const LPVOID msg, const DWORD len) const;
	bool ReplyProcess(const FILTER_PROCESS_CHECK_REPLY& msg) const;
	bool ReplyKey(const LPVOID msg, const DWORD len) const;


private:
	bool StartDriver();
	bool StopDriver();

	bool Connect();
	void Disconnect();

	void ReleaseHandle();

	void FilterWorker();
	void FilterWorkerNotify();
	bool HandleFilterMsg(ULONG_PTR key, LPOVERLAPPED pOvlp);
	bool HandleFilterNotifyMsg(ULONG_PTR key, LPOVERLAPPED pOvlp);

	bool SendToPort(HANDLE port, const LPVOID msg, const DWORD len) const;
	bool ReplyToPort(HANDLE port, const LPVOID msg, const DWORD len) const;

	bool TimeOut(ULONGLONG time_stamp) const
	{
		auto time_now = Utils::GetTickCount(); //milliseconds
		if (time_now - time_stamp >= FILTER_TIME_OUT)
		{
			return true;
		}

		return false;
	}

private:
	HANDLE main_port_ = INVALID_HANDLE_VALUE;
	HANDLE key_port_ = INVALID_HANDLE_VALUE;
	HANDLE process_check_port_ = INVALID_HANDLE_VALUE;
	HANDLE completion_ = NULL;

	HANDLE notify_port_ = INVALID_HANDLE_VALUE;
	HANDLE completion_notify_ = NULL;

	std::vector<std::thread> threads_;
	std::thread thread_notify;
	HeartBeat heart_beater_;
};


#endif // 
