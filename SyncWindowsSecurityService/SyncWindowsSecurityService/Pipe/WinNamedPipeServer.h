#ifndef WinNamedPipeServer_h__
#define WinNamedPipeServer_h__


#include <Windows.h>
#include <string>
#include <boost/thread.hpp>


class WinNamedPipeServer
{
public:
	WinNamedPipeServer();

	// Ӧ����һ���߳���ȥ�������
	void Run(const std::string& pipeName);
	// 
	bool Send(const std::string& data);

private:

private:
	boost::mutex mio;
	HANDLE m_hNamedPipe;

};


#endif // WinNamedPipeServer_h__
