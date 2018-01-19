#ifndef WindowsServiceThread_h__
#define WindowsServiceThread_h__


#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include "../Pipe/WinNamedPipe.h"
#include "../driver/msg.h"
#include "../Logger/BoostLog.h"
#include "../Util/Utils.h"
#include "../driver/Strategy.h"

class WindowsServiceCtrlThread : public boost::noncopyable
{
public:
	static WindowsServiceCtrlThread& GetInstance();
	void Run();
	void Interrupt();
	void Join();

private:
	WindowsServiceCtrlThread();
	void Ctrl();

private:
	static WindowsServiceCtrlThread *instance_;

private:
	boost::thread* threadPtr_;
};


// 可以写成单实例
class WindowsMonitorServiceThread : public boost::noncopyable
{
public:
	static WindowsMonitorServiceThread& GetInstance();
	void Run();
	void Interrupt();
	void Join();

private:
	WindowsMonitorServiceThread();
	void Monitor();

private:
	static WindowsMonitorServiceThread *instance_;

private:
	//boost::shared_ptr<boost::thread> threadPtr_;
	boost::thread* threadPtr_;
};

#include "../Util/Utils.h"
class WindowsNotificationThread
{
public:
	static WindowsNotificationThread& GetInstance();
	void Run();
	void Join();
	void Add(const DRIVER_NOTIFICATION& msg);

	struct Operation
	{
		unsigned int operation_type;
		DWORD process_id;
		std::wstring file_path;
		std::wstring process_name;
	};

private:
	WindowsNotificationThread() {};
	void listen();



	class Filter
	{
	public:
		bool do_filter(const Operation& op)
		{
			if (Utils::IsSysProcess(op.process_name))
			{
				SLOG_DEBUG << "notification filter out: system process: " << op.process_name;
				return false;
			}
			else if (Utils::IsAntivirusProcess(op.process_name))
			{
				SLOG_DEBUG << "notification filter out: anti-virus process: " << op.process_name;
				return false;
			}
			else if (is_duplicate(op.process_name, op.file_path, op.operation_type)
				&&is_in_time_limit())
			{
				try
				{
					SLOG_DEBUG << "notification filter out: time filter in or duplicate" << last_process_ << " " << last_file_name_ << " " << operation_type_;
				}
				catch (const std::exception&)
				{

				}
				return false;
			}
			else if (!SecurityStrategy::Instance()->IsWhiteProcess(op.process_name) &&
				Utils::IsFolder(op.file_path) && (op.operation_type != OPERATE_MOVE_OUT && op.operation_type != OPERATE_RENAME_PATH))
			{
				//SLOG_DEBUG << "notification filter out: folder: " << op.file_path;
				return false;
			}

			if (Utils::IsFangCloud(op.process_name) || Utils::IsSecuritySvr(op.process_name))
			{
				return false;
			}

			last_process_ = op.process_name;
			last_file_name_ = op.file_path;
			operation_type_ = op.operation_type;
			last_tm_ = std::chrono::steady_clock::now();
			return true;
		}

		bool is_duplicate(const std::wstring& name, const std::wstring& file, const int op )
		{
			return last_process_ == name && last_file_name_ == file && operation_type_ == op;
		}

		bool is_in_time_limit()
		{
			now_tm_ = std::chrono::steady_clock::now();
			auto time_interval = std::chrono::duration_cast<std::chrono::milliseconds>(now_tm_ - last_tm_).count();

			if (time_interval <= 1500)
			{
				try
				{
					SLOG_DEBUG << "time filter in: " << last_process_ << " " << last_file_name_ << " " << operation_type_ << " time interval: " << time_interval;
				}
				catch (const std::exception&)
				{

				}

				return true;
			}

			return false;
		}

	private:
		std::wstring last_process_ = L"";
		std::wstring last_file_name_ = L"";
		int operation_type_ = -1;
		std::chrono::steady_clock::time_point last_tm_ = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point now_tm_;
	};
private:
	std::thread t_;
	std::shared_ptr<WinNamedPipe> pipe_;
	std::vector<Operation> incoming_queue_;
	std::vector<Operation> queue_;
	std::mutex m_;
	bool exit_ = false;
	Filter filter_;
};


#endif // WindowsServiceThread_h__
