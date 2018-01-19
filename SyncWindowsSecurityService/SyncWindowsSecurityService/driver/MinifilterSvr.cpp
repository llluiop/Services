#include "MinifilterSvr.h"
#include "../Logger/BoostLog.h"
#include "../Util/ScopeGuard.h"
#include "../Util/WinService.h"
#include "../Frame/WindowsServiceThread.h"
#include "../Encrypted/key/FileKey.h"
#include "MsgHandler.h"
#include "strategy.h"
#include <tchar.h>
#include <fltuser.h>

using namespace std;
using namespace boost;

#define FILTER_NAME							_T("FangCloud")
#define FILTER_MAINPORT						_T("\\FangYunPort")
#define FILTER_NOTIFY_PORT					_T("\\FangNotifyPort")
#define FANGCLOUD_KEY_PORT					_T("\\ItppdACCoKK")
#define FANGCLOUD_PROCESS_PORT				_T("\\OoIrpndopr")
#define FILTER_DEFAULT_REQUEST_COUNT        5
#define FILTER_HEARTBEAT_INTERVAL           1

MinifilterSvr::MinifilterSvr()
	:heart_beater_(FILTER_HEARTBEAT_INTERVAL)
{

}

MinifilterSvr::~MinifilterSvr()
{

}

bool MinifilterSvr::Init()
{
	if (!StartDriver())
	{
		return false;
	}

	if (!Connect())
	{
		return false;
	}

	//heart-beat sent to minifilter every SCANNER_HEARTBEAT_INTERVAL seconds
	heart_beater_.Start([&] {
		HEART_BEAT heart_beat;
		heart_beat.Header.ulMessageID = MSG_HEARTBEAT;

		SendHeartBeat(&heart_beat, sizeof(HEART_BEAT));
	});

	SLOG_INFO << "Suc start";
	return true;
}

bool MinifilterSvr::UnInit()
{
	heart_beater_.Stop();

	Disconnect();

	if (!StopDriver())
	{
		return false;
	}

	SLOG_INFO << "Suc stop";
	return true;
}

bool MinifilterSvr::StartDriver()
{
	StopDriver();
	return WinService::StartWinService(FILTER_NAME);
}

bool MinifilterSvr::StopDriver()
{
	return WinService::StopWinService(FILTER_NAME);
}

bool MinifilterSvr::Connect()
{
	HRESULT hr = FilterConnectCommunicationPort(FILTER_MAINPORT, 0, NULL, 0, NULL, &main_port_);
	if (IS_ERROR(hr))
	{
		SLOG_ERROR << boost::format("Filter: Connecting to filter port: 0x%08x") % hr;
		return false;
	}

	completion_ = CreateIoCompletionPort(main_port_, NULL, (ULONG_PTR)main_port_, Utils::ConcurrencyAbility());
	if (completion_ == NULL)
	{
		SLOG_ERROR << boost::format("Filter: Creating completion port: %d") % GetLastError();
		CloseHandle(main_port_);
		return false;
	}
	SLOG_INFO << boost::format("Filter: Port = 0x%p Completion = 0x%p") % main_port_ % completion_;


	hr = FilterConnectCommunicationPort(FANGCLOUD_KEY_PORT, 0, NULL, 0, NULL, &key_port_);
	if (SUCCEEDED(hr))
	{
		completion_ = CreateIoCompletionPort(key_port_, completion_, (ULONG_PTR)key_port_, Utils::ConcurrencyAbility());
		if (completion_ == NULL)
		{
			SLOG_ERROR << boost::format("ERROR: Creating key completion port: %d") % GetLastError();
			CloseHandle(key_port_);
		}

		SLOG_INFO << boost::format("Filter: Creating key completion port: 0x%p") % key_port_;
	}

	hr = FilterConnectCommunicationPort(FANGCLOUD_PROCESS_PORT, 0, NULL, 0, NULL, &process_check_port_);
	if (SUCCEEDED(hr))
	{
		completion_ = CreateIoCompletionPort(process_check_port_, completion_, (ULONG_PTR)process_check_port_, Utils::ConcurrencyAbility());
		if (completion_ == NULL)
		{
			SLOG_ERROR << boost::format("ERROR: Creating process_check_port_ completion port: %d") % GetLastError();
			CloseHandle(process_check_port_);
		}

		SLOG_INFO << boost::format("Filter: Creating process_check_port_ completion port: 0x%p") % process_check_port_;
	}

	hr = FilterConnectCommunicationPort(FILTER_NOTIFY_PORT, 0, NULL, 0, NULL, &notify_port_);
	if (SUCCEEDED(hr))
	{
		completion_notify_ = CreateIoCompletionPort(notify_port_, nullptr, (ULONG_PTR)notify_port_, 1); //use single thread
		if (completion_notify_ == NULL)
		{
			SLOG_ERROR << boost::format("ERROR: Creating notify completion port: %d") % GetLastError();
			CloseHandle(notify_port_);
		}

		SLOG_INFO << boost::format("Filter: Creating notify completion port = 0x%p Completion = 0x%p") % notify_port_ % completion_notify_;
	}

	return true;
}

void MinifilterSvr::Disconnect()
{
	//close existing port
	ReleaseHandle();
}

bool MinifilterSvr::Listen()
{
	for (int i = 0; i < Utils::ConcurrencyAbility(); i++)
	{
		threads_.push_back(std::thread(&MinifilterSvr::FilterWorker, this));


		for (int i = 0; i < FILTER_DEFAULT_REQUEST_COUNT; i++)
		{
			PFILTER_PROCESS_CHECK_MESSAGE msg = new FILTER_PROCESS_CHECK_MESSAGE;
			memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

			//get msg from minifilter async
			HRESULT hr = FilterGetMessage(process_check_port_,
				&msg->MessageHeader,
				FIELD_OFFSET(FILTER_PROCESS_CHECK_MESSAGE, Ovlp),
				&msg->Ovlp);

			if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			{
				SLOG_ERROR << boost::format("Filter: FilterGetMessage = 0x%x") % hr;

				ReleaseHandle();
				delete msg;
			}
		}

		for (int i = 0; i < FILTER_DEFAULT_REQUEST_COUNT; i++)
		{
			PFILTER_GET_KEY_MESSAGE msg = new FILTER_GET_KEY_MESSAGE;
			memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

			//get msg from minifilter async
			HRESULT hr = FilterGetMessage(key_port_,
				&msg->MessageHeader,
				FIELD_OFFSET(FILTER_GET_KEY_MESSAGE, Ovlp),
				&msg->Ovlp);

			if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			{
				SLOG_ERROR << boost::format("Filter: FilterGetMessage = 0x%x") % hr;

				ReleaseHandle();
				delete msg;
			}
		}

	}

	for (auto& t : threads_)
	{
		if (::SetThreadPriority(t.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL))
		{
			SLOG_INFO << "SetThreadPriority for " << t.get_id();
		}
	}

	thread_notify = std::thread(&MinifilterSvr::FilterWorkerNotify, this);
	for (int i = 0; i < FILTER_DEFAULT_REQUEST_COUNT; i++)
	{
		PFILTER_NOTIFICATION_MESSAGE msg = new FILTER_NOTIFICATION_MESSAGE;
		memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

		//get msg from minifilter async
		HRESULT hr = FilterGetMessage(notify_port_,
			&msg->MessageHeader,
			FIELD_OFFSET(FILTER_NOTIFICATION_MESSAGE, Ovlp),
			&msg->Ovlp);

		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			SLOG_ERROR << boost::format("Filter: Notification FilterGetMessage = 0x%x") % hr;

			ReleaseHandle();
			delete msg;
		}
	}
	return true;
}



void MinifilterSvr::Stop()
{
	for (auto& t : threads_)
	{
		//notify thread to quit
		if (PostQueuedCompletionStatus(completion_, 0, 0, nullptr))
		{
			SLOG_INFO << "PostQueuedCompletionStatus for " << t.get_id();
		}
	}

	for (auto& t : threads_)
	{
		t.join();
	}

	//notify thread to quit
	if (PostQueuedCompletionStatus(completion_notify_, 0, 0, nullptr))
	{
		SLOG_INFO << "PostQueuedCompletionStatus for " << thread_notify.get_id();
	}

	if (thread_notify.joinable())
	{
		thread_notify.join();
	}

	SLOG_INFO << "MinifilterSvr end";
}


bool MinifilterSvr::SendHeartBeat(const LPVOID msg, const DWORD len) const
{
	return SendToPort(main_port_, msg, len);
}

bool MinifilterSvr::SendSecurityPath(const LPVOID msg, const DWORD len) const
{
	return SendToPort(main_port_, msg, len);
}

bool MinifilterSvr::ReplyProcess(const FILTER_PROCESS_CHECK_REPLY& msg) const
{
	if (TimeOut(*(ULONGLONG*)msg.req.Result.llTimeStamp))
	{
		SLOG_WARNING << "time out";
		return false;
	}

	return ReplyToPort(process_check_port_, (LPVOID)&msg, sizeof(FILTER_REPLY_HEADER) + sizeof(CHECK_PROCESS_RSP));
}


bool MinifilterSvr::ReplyKey(const LPVOID msg, const DWORD len) const
{
	return ReplyToPort(key_port_, msg, len);
}

bool MinifilterSvr::SendToPort(HANDLE port, const LPVOID msg, const DWORD len) const
{
	DWORD ret = 0;
	DRIVER_RESPONESE res;

	HRESULT hr = FilterSendMessage(port, msg, len, &res, sizeof(DRIVER_RESPONESE), &ret);
	if (!SUCCEEDED(hr))
	{
		SLOG_ERROR << boost::format("Scanner: Error send message. Error = 0x%x") % hr;
		return false;
	}

	return true;
}

bool MinifilterSvr::ReplyToPort(HANDLE port, const LPVOID msg, const DWORD len) const
{
	HRESULT hr = FilterReplyMessage(port, (PFILTER_REPLY_HEADER)msg, len);
	if (!SUCCEEDED(hr))
	{
		SLOG_ERROR << boost::format("Scanner: Error reply message. Error = 0x%x") % hr;
		return false;
	}

	return true;
}

void MinifilterSvr::ReleaseHandle()
{
	CloseHandle(main_port_);
	CloseHandle(process_check_port_);
	CloseHandle(key_port_);
	CloseHandle(notify_port_);
	CloseHandle(completion_);
	CloseHandle(completion_notify_);

	SLOG_INFO << boost::format("ReleaseHandle port = %1%, log port = %2%, notification port = %3%, completion = %4%") % main_port_ %key_port_ % notify_port_ % completion_;
}



void MinifilterSvr::FilterWorker()
{
	SLOG_INFO << "worker runs with id = " << std::this_thread::get_id();

	LPOVERLAPPED pOvlp = nullptr;
	DWORD outSize = 0; 
	ULONG_PTR key = 0;
	while (TRUE)
	{
		BOOL result = GetQueuedCompletionStatus(completion_, &outSize, &key, &pOvlp, INFINITE);
		if (!result) 
		{
			auto d = GetLastError();
			SLOG_ERROR << "GetQueuedCompletionStatus Failed:"  << GetLastError();
			break;
		}

		//quit signal received
		if (outSize == 0 && key == 0 && pOvlp == nullptr)
		{
			SLOG_INFO << boost::format("FilterWorker exits %d") % std::this_thread::get_id();
			break;
		}

		if (!HandleFilterMsg(key, pOvlp))
		{
			break;
		}
	}

	SLOG_INFO << "worker ends with id = " << std::this_thread::get_id();

}

void MinifilterSvr::FilterWorkerNotify()
{
	SLOG_INFO << "notify worker runs with id = " << std::this_thread::get_id();

	LPOVERLAPPED pOvlp = nullptr;
	DWORD outSize = 0;
	ULONG_PTR key = 0;
	while (TRUE)
	{
		BOOL result = GetQueuedCompletionStatus(completion_notify_, &outSize, &key, &pOvlp, INFINITE);
		if (!result)
		{
			auto d = GetLastError();
			SLOG_ERROR << "GetQueuedCompletionStatus Failed:" << GetLastError();
			break;
		}

		//quit signal received
		if (outSize == 0 && key == 0 && pOvlp == nullptr)
		{
			SLOG_INFO << boost::format("FilterWorker exits %d") % std::this_thread::get_id();
			break;
		}

		if (!HandleFilterNotifyMsg(key, pOvlp))
		{
			break;
		}
	}

	SLOG_INFO << "worker ends with id = " << std::this_thread::get_id();

}



bool MinifilterSvr::HandleFilterMsg(ULONG_PTR key, LPOVERLAPPED pOvlp)
{
	if (key == (ULONG_PTR)process_check_port_)
	{
		PFILTER_PROCESS_CHECK_MESSAGE message = CONTAINING_RECORD(pOvlp, FILTER_PROCESS_CHECK_MESSAGE, Ovlp);
		if (message == nullptr)
		{
			SLOG_WARNING << "Empty Message";
			return true;
		}

		//to-do:handle process check
		auto id = message->req.Param.uiProcessID;

		FILTER_PROCESS_CHECK_REPLY reply;

		reply.MessageHeader.Status = 0;
		reply.MessageHeader.MessageId = message->MessageHeader.MessageId;
		reply.req.Result.uiProcType = MsgHandler::GetInstance()->GetProcessType(Utils::GetProcessName(id));
		reply.req.Result.uiResult = OP_RESULT_SUCCESS;
		reply.req.Result.lSequenceID = message->req.Param.lSequenceID;
		memcpy(reply.req.Result.llTimeStamp, message->req.Param.llTimeStamp, sizeof(reply.req.Result.llTimeStamp));


		ReplyProcess(reply);

		//SLOG_INFO << "FILTER_PROCESS_CHECK_REPLY: process id: " << id << "type: " << reply.req.Result.uiProcType;


		memset(&message->Ovlp, 0, sizeof(OVERLAPPED));

		HRESULT hr = FilterGetMessage(process_check_port_, &message->MessageHeader, FIELD_OFFSET(FILTER_PROCESS_CHECK_MESSAGE, Ovlp), &message->Ovlp);

		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			SLOG_ERROR << boost::format("Filter: Unknown error occurred. Error = 0x%x") % hr;
		}
	}
	else if (key == (ULONG_PTR)key_port_)
	{
		PFILTER_GET_KEY_MESSAGE message = CONTAINING_RECORD(pOvlp, FILTER_GET_KEY_MESSAGE, Ovlp);
		if (message == nullptr)
		{
			SLOG_WARNING << "Empty Message";
			return true;
		}

		//to-do:handle get key check
		if (TimeOut(*(ULONGLONG*)message->req.Param.llTimeStamp))
		{
			SLOG_WARNING << "key time out: " << message->req.Param.lSequenceID;
		}
		else
		{
			SLOG_DEBUG << "request key: " << message->req.Param.lSequenceID;

			auto file_id = message->req.Param.llFileID;
			auto key_block = FileKey::Instance()->GetKey(string((char*)file_id, MAX_KEY_LENGTH));

			FILTER_GET_KEY_REPLY reply;

			reply.MessageHeader.Status = 0;
			reply.MessageHeader.MessageId = message->MessageHeader.MessageId;
			reply.req.Result.uiResult = OP_RESULT_SUCCESS;
			reply.req.Result.lSequenceID = message->req.Param.lSequenceID;
			memcpy(reply.req.Result.llFileID, file_id, sizeof(reply.req.Result.llFileID));
			memcpy(reply.req.Result.ucKey, key_block.key.c_str(), sizeof(reply.req.Result.ucKey));
			memcpy(reply.req.Result.ucIV, key_block.iv.c_str(), sizeof(reply.req.Result.ucIV));
			memcpy(reply.req.Result.llTimeStamp, message->req.Param.llTimeStamp, sizeof(reply.req.Result.llTimeStamp));

			ReplyKey(&reply, sizeof(FILTER_REPLY_HEADER) + sizeof(GET_KEY_RSP));
		}

		memset(&message->Ovlp, 0, sizeof(OVERLAPPED));
		HRESULT hr = FilterGetMessage(key_port_, &message->MessageHeader, FIELD_OFFSET(FILTER_GET_KEY_MESSAGE, Ovlp), &message->Ovlp);

		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			SLOG_ERROR << boost::format("Filter: Unknown error occurred. Error = 0x%x") % hr;
		}
	}

	return true;
}

bool MinifilterSvr::HandleFilterNotifyMsg(ULONG_PTR key, LPOVERLAPPED pOvlp)
{
	PFILTER_NOTIFICATION_MESSAGE message = CONTAINING_RECORD(pOvlp, FILTER_NOTIFICATION_MESSAGE, Ovlp);
	if (message == nullptr)
	{
		SLOG_WARNING << "Empty Message";
		return true;
	}

	WindowsNotificationThread::GetInstance().Add(message->notification.Param);

	memset(&message->Ovlp, 0, sizeof(OVERLAPPED));


	HRESULT hr = FilterGetMessage(notify_port_, &message->MessageHeader, FIELD_OFFSET(FILTER_NOTIFICATION_MESSAGE, Ovlp), &message->Ovlp);

	if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
	{
		SLOG_ERROR << boost::format("Filter: Unknown error occurred. Error = 0x%x") % hr;
	}
	

	return true;
}


