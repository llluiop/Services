#include "WindowsServiceThread.h"
#include "../Pipe/PipeServerManager.h"
#include "../Logger/BoostLog.h"
#include "../Util/Utils.h"
#include "../driver/strategy.h"
#include "../Inject/injector.h"
#include <boost/function.hpp>
#include <locale>
#include <codecvt>

using namespace std;
//using namespace boost;


WindowsServiceCtrlThread *WindowsServiceCtrlThread::instance_ = NULL;


WindowsServiceCtrlThread& WindowsServiceCtrlThread::GetInstance()
{
	if (NULL == instance_)
		instance_ = new WindowsServiceCtrlThread;

	return *instance_;
}


WindowsServiceCtrlThread::WindowsServiceCtrlThread()
{
	threadPtr_ = NULL;
}


void WindowsServiceCtrlThread::Interrupt()
{
	if (NULL != threadPtr_)
	{
		threadPtr_->interrupt();
	}

}


void WindowsServiceCtrlThread::Join()
{
	threadPtr_->join();
}


void WindowsServiceCtrlThread::Ctrl()
{
	try
	{
		while (true)
		{
			boost::this_thread::interruption_point();
			boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));
		}
	}
	catch (boost::thread_interrupted&)
	{
		return;
	}

}


void WindowsServiceCtrlThread::Run()
{
	boost::function<void ()> monitorProcess = bind(&WindowsServiceCtrlThread::Ctrl, this);
	threadPtr_ = new boost::thread(monitorProcess);

}


WindowsMonitorServiceThread *WindowsMonitorServiceThread::instance_ = NULL;


WindowsMonitorServiceThread& WindowsMonitorServiceThread::GetInstance()
{
	if (NULL == instance_)
		instance_ = new WindowsMonitorServiceThread;

	return *instance_;
}


WindowsMonitorServiceThread::WindowsMonitorServiceThread()
{
	threadPtr_ = NULL;
}


void WindowsMonitorServiceThread::Interrupt()
{
	if (NULL != threadPtr_)
	{
		threadPtr_->interrupt();
	}
	
}


void WindowsMonitorServiceThread::Join()
{
	threadPtr_->join();
}


void WindowsMonitorServiceThread::Monitor()
{
	PipeServerManager& pipeMgr = PipeServerManager::GetInstance();
	pipeMgr.Run();

}


void WindowsMonitorServiceThread::Run()
{
	boost::function<void ()> monitorProcess = bind(&WindowsMonitorServiceThread::Monitor, this);
	threadPtr_ = new boost::thread(monitorProcess);
	
}


WindowsNotificationThread& WindowsNotificationThread::GetInstance()
{
	static WindowsNotificationThread instance;
	return instance;
}

void WindowsNotificationThread::Run()
{
	pipe_.reset(new WinNamedPipe);
	if (!pipe_->Run("\\\\.\\pipe\\securitypipename"))
	{
		SLOG_ERROR << "WindowsNotificationThread Run failed";
	}
#ifdef SINGLE_TEST
	pipe_->Send("1|C:\\FangCloudV2\\sa1\\123\\ssa\\1111.xlsx|1\\test.exe*");
	//pipe_->Send("5");
	//pipe_->Send("6@1\\test.exe");
	//pipe_->Send("2@1\\test.exe");
	//pipe_->Send("1@1\\test.exe");
#endif
	t_ = std::thread(std::bind(&WindowsNotificationThread::listen, this));
}

void WindowsNotificationThread::Join()
{
	exit_ = true;
	if (t_.joinable())
	{
		t_.join();
	}
}

void WindowsNotificationThread::Add(const DRIVER_NOTIFICATION& msg)
{
	Operation op = { msg.uiOpType, msg.hProcessID, msg.wsFilePath, Utils::GetProcessName(msg.hProcessID) };

	if (op.process_name == _T(""))
	{
		SLOG_ERROR << "WindowsNotificationThread add process name null: " << msg.hProcessID;
		return;
	}

	std::lock_guard<std::mutex> lck(m_);
	if (filter_.do_filter(op))
	{
		//SLOG_INFO << "add notification" << op.process_name << " " << op.file_path << " " << op.operation_type;
		incoming_queue_.push_back(op);
	}
}




string make_msg(const wstring& type, const WindowsNotificationThread::Operation& op)
{
	wstring msg = type + _T("|") + op.file_path + _T("|") + op.process_name + _T("*");

	try
	{
		if (type != _T("6"))
		{
			SLOG_INFO << "make_msg: " << msg;
		}

	}
	catch (const std::exception&)
	{
		
	}

	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.to_bytes(msg);
}

void WindowsNotificationThread::listen()
{
	if (pipe_ == nullptr)
	{
		return;
	}

	SLOG_INFO << "WindowsNotificationThread start";

	while (true)
	{
		{
			std::lock_guard<std::mutex> lck(m_);
			queue_.swap(incoming_queue_);
		}

		for (auto q : queue_)
		{
			//no explorer notify anymore

			//black list
			if (!SecurityStrategy::Instance()->IsWhiteProcess(q.process_name))
			{
				if ((q.operation_type == OPERATE_OPEN || q.operation_type == OPERATE_COPY) //send by im
					&& Utils::IsIM(q.process_name))
				{ 
					pipe_->Send(make_msg(_T("3"), q));
				}
				else if (q.operation_type == OPERATE_OPEN || q.operation_type == OPERATE_COPY) //blacklist open
				{
					pipe_->Send(make_msg(_T("2"), q));
				}
			}
			else if (SecurityStrategy::Instance()->IsWhiteProcess(q.process_name))
			{
				if (q.operation_type == OPERATE_OPEN) //whilte open
				{
					//inject 
					Injector::Inject(q.process_id, q.process_name);

					//notify
					pipe_->Send(make_msg(_T("6"), q));
				}
			}

		}

		queue_.clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		if (exit_ == true)
		{
			break;
		}
	}

	SLOG_INFO << "WindowsNotificationThread end";

}
