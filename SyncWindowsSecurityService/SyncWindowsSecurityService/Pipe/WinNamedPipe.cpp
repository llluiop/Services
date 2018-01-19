#include "WinNamedPipe.h"
//#include "Dispatcher.h"
#include "../Logger/BoostLog.h"

using namespace std;


WinNamedPipe::WinNamedPipe()
	:m_hNamedPipe(NULL)
	,pipe_name_("")
{

}


WinNamedPipe::~WinNamedPipe()
{
	if (NULL != m_hNamedPipe && INVALID_HANDLE_VALUE != m_hNamedPipe)
	{
		CloseHandle(m_hNamedPipe);
		m_hNamedPipe = NULL;
	}
}


void WinNamedPipe::Connect()
{
	// 绑定一个用户回调？
	 
}


bool WinNamedPipe::IsConnected()
{
	boost::lock_guard<boost::mutex> lock(mio);
	if (INVALID_HANDLE_VALUE == m_hNamedPipe || NULL == m_hNamedPipe)
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool WinNamedPipe::IsConnectedWithoutLock()
{
	if (INVALID_HANDLE_VALUE == m_hNamedPipe || NULL == m_hNamedPipe)
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool WinNamedPipe::Send(const string&strContent)
{
	DWORD dwCurWritedBytes = 0;
	while (dwCurWritedBytes < strContent.length())
	{
		DWORD dwWrite = 0;
		//向命名管道中写入数据

		BOOL errorFlag = FALSE;
		{
			//boost::lock_guard<boost::mutex> lock(mio);
			errorFlag = WriteFile(m_hNamedPipe, (LPCVOID)(strContent.c_str() + dwCurWritedBytes), 
				strContent.length() - dwCurWritedBytes, &dwWrite, NULL);
		}
		
		if (FALSE == errorFlag)
		{
			m_hNamedPipe = NULL;
			SLOG_ERROR << "WaitNamedPipe send failed: " << GetLastError();
			Run(pipe_name_);
			return false;
		}

		dwCurWritedBytes += dwWrite;
	}

	return true;
}


bool WinNamedPipe::Run(const string& strPipeName)
{
	// here may many thread entered,so use mutex to avoid pipe multi conflict
	boost::lock_guard<boost::mutex> lock(mio);
	if (!IsConnectedWithoutLock())
	{
		// 等待连接命名管道
		if (!WaitNamedPipeA(strPipeName.c_str(), 10000))
		{
			SLOG_ERROR << "WaitNamedPipe failed: " << strPipeName;
			return false;
		}

		// 
		m_hNamedPipe = CreateFileA(strPipeName.c_str(), 
			GENERIC_WRITE, 
			0, 
			NULL, 
			OPEN_EXISTING, 
			FILE_FLAG_OVERLAPPED, 
			NULL);

		if (INVALID_HANDLE_VALUE == m_hNamedPipe)
		{
			SLOG_ERROR << "WinNamedPipe Run Error:" << GetLastError();
			return false;
		}

	}

	pipe_name_ = strPipeName;
	return true;
}

std::string WinNamedPipe::SendWithReply(const std::string&strContent)
{
	DWORD dwWrite = 0;
	DWORD dwRead = 0;

	//std::lock_guard<std::mutex> lck(m_); //in KeyBlock

	//向命名管道中写入数据
	BOOL errorFlag = FALSE;
	{
		//boost::lock_guard<boost::mutex> lock(mio);
		errorFlag = WriteFile(m_hNamedPipe, (LPCVOID)(strContent.c_str()),
			strContent.length(), &dwWrite, NULL);
	}


	if (FALSE == errorFlag)
	{
		SLOG_ERROR << "WaitNamedPipe write failed: " << GetLastError();
		return "";
	}


	char buff[128] = { 0 };
	errorFlag = ReadFile(m_hNamedPipe, buff, 128, &dwRead, nullptr);
	
	SLOG_DEBUG << "read file bytes: " << dwRead;

	if (FALSE == errorFlag)
	{
		SLOG_ERROR << "WaitNamedPipe read failed: " << GetLastError();
		return "";
	}

	return std::string(buff, dwRead);
}

bool WinNamedPipe::RunSync(const std::string&strPipeName)
{
	// here may many thread entered,so use mutex to avoid pipe multi conflict
	boost::lock_guard<boost::mutex> lock(mio);
	if (!IsConnectedWithoutLock())
	{
		// 等待连接命名管道
		if (!WaitNamedPipeA(strPipeName.c_str(), 10000))
		{
			SLOG_ERROR << "WaitNamedPipe failed: " << strPipeName;
			return false;
		}

		// 
		m_hNamedPipe = CreateFileA(strPipeName.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (INVALID_HANDLE_VALUE == m_hNamedPipe)
		{
			SLOG_ERROR << "WinNamedPipe Run Error:" << GetLastError();
			return false;
		}

	}

	pipe_name_ = strPipeName;
	return true;
}

