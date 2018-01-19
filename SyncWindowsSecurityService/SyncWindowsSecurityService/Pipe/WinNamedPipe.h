#ifndef WinNamedPipe_h__
#define WinNamedPipe_h__


#include <Windows.h>
#include <string>
#include <mutex>
#include <boost/thread.hpp>


// 可能在多线程中使用，特别是m_hNamedPipe变量

class WinNamedPipe{
public:
	WinNamedPipe();
	~WinNamedPipe();

	void Connect();

	bool IsConnected();
	bool IsConnectedWithoutLock();
	bool Send(const std::string&strContent);
	bool Run(const std::string&strPipeName);

	std::string SendWithReply(const std::string&strContent);
	bool RunSync(const std::string&strPipeName);

private:
	boost::mutex mio;
	std::mutex m_;
	HANDLE m_hNamedPipe;
	std::string pipe_name_;
};


#endif // WinNamedPipe_h__
