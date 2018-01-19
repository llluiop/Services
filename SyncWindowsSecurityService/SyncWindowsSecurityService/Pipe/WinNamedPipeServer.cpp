#include "WinNamedPipeServer.h"
#pragma warning (disable: 4995)
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>
#include <iostream>
#include <string>

#include "../Logger/BoostLog.h"
#include "../Json/RequestJsonOperator.h"
#include "../Frame/WindowsServiceEventControl.h"

//#define BUFSIZE 512
//#define BUFSIZE 4096
#define BUFSIZE 131072

extern bool g_InitFlag;
HANDLE g_CurPipe = INVALID_HANDLE_VALUE;

VOID GetAnswerToRequest(char*, DWORD); 


DWORD WINAPI InstanceThread(LPVOID lpvParam)
{ 
	HANDLE hHeap      = GetProcessHeap();
	char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(char));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0; 
	BOOL fSuccess = FALSE;
	HANDLE hPipe  = NULL;

	// Do some extra error checking since the app will keep running even if this
	// thread fails.

	if (lpvParam == NULL)
	{
		SLOG_INFO<<"\nERROR - Pipe Server Failure:\n";
		SLOG_INFO<<"InstanceThread got an unexpected NULL value in lpvParam.\n";
		SLOG_INFO<<"InstanceThread exitting.\n";
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		SLOG_INFO<<"\nERROR - Pipe Server Failure:\n";
		SLOG_INFO<<"InstanceThread got an unexpected NULL heap allocation.\n";
		SLOG_INFO<<"InstanceThread exitting.\n";
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	SLOG_INFO<<"InstanceThread created, receiving and processing messages.\n";

	// The thread's parameter is a handle to a pipe object instance. 

	hPipe = (HANDLE) lpvParam; 
	g_CurPipe = hPipe;

	// Loop until done reading
	while (1) 
	{ 
		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		fSuccess = ReadFile( 
			hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE*sizeof(char), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!fSuccess/* || cbBytesRead == 0*/)
		{   
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE)
			{
				SLOG_INFO<< "InstanceThread: client disconnected.\n" << error; 
			}
			else
			{
				SLOG_INFO<< "InstanceThread ReadFile failed, GLE=" << error; 
			}
			break;
		}

		// Process the incoming message.
		GetAnswerToRequest(pchRequest, cbBytesRead); 
	}

	FlushFileBuffers(hPipe); 
	DisconnectNamedPipe(hPipe); 
	CloseHandle(hPipe); 

	HeapFree(hHeap, 0, pchRequest);

	SLOG_INFO<<"InstanceThread exitting.\n";
	return 1;
}


WinNamedPipeServer::WinNamedPipeServer() : m_hNamedPipe(INVALID_HANDLE_VALUE)
{
	
}


void WinNamedPipeServer::Run(const std::string& pipeName)
{
	DWORD  dwThreadId = 0; 
	HANDLE hThread = NULL; 
	HANDLE hNamedPipe = INVALID_HANDLE_VALUE;

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and this loop is free to wait for the
	// next client connect request. It is an infinite loop.
	try
	{
		for (;;) 
			{ 
				// 更改pipe的权限
				SECURITY_DESCRIPTOR sd;
				InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
				SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

				SECURITY_ATTRIBUTES sa = {0};
				sa.nLength = sizeof(SECURITY_ATTRIBUTES);
				sa.lpSecurityDescriptor = &sd;
				sa.bInheritHandle = FALSE;

				hNamedPipe = CreateNamedPipeA( 
					pipeName.c_str(),         // pipe name 
					PIPE_ACCESS_DUPLEX |      // read/write access 
					FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_WAIT,                // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances  
					BUFSIZE,                  // output buffer size 
					BUFSIZE,                  // input buffer size 
					0,						  // client time-out 
					//NULL					  // default security attribute 
					&sa);                    


				if (hNamedPipe == INVALID_HANDLE_VALUE) 
				{
					int errorCode = GetLastError();
					SLOG_INFO<<"CreateNamedPipe failed, GLE="<<errorCode; 
					return;
				}

				SLOG_INFO << "monitor server pipe created.";

				// Wait for the client to connect; if it succeeds, 
				// the function returns a nonzero value. If the function
				// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
				BOOL fConnected = ConnectNamedPipe(hNamedPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

				if (fConnected) 
				{ 
					SLOG_INFO<< "Client connected, creating a processing thread.\n";

					// Create a thread for this client. 
					hThread = CreateThread( 
						NULL,              // no security attribute 
						0,                 // default stack size 
						InstanceThread,    // thread proc
						(LPVOID) hNamedPipe,    // thread parameter 
						0,                 // not suspended 
						&dwThreadId);      // returns thread ID 

					if (hThread == NULL) 
					{
						int errorCode = GetLastError();
						SLOG_INFO<<"CreateThread failed, GLE="<<errorCode;
		
						return;
					}
					else CloseHandle(hThread); 
				}  
				else 
				{
					// The client could not connect, so close the pipe. 
					SLOG_INFO << "monitor server pipe close because the client could not connect.";
					CloseHandle(hNamedPipe); 
				}
			} 
	}
	catch (boost::thread_interrupted& )
	{
		// 可以在这里加上一些线程中断退出的操作
		SLOG_INFO << "catch thread_interrupted exception.";
		return;
	}
	catch (...)
	{
		SLOG_INFO << "finally i catch you in pipe server thread.";
		return;
	}
	
	return; 
}


bool WinNamedPipeServer::Send(const std::string&strContent)
{
	DWORD dwCurWritedBytes = 0;
	// use overlapped io
	bool result = true;

	while (dwCurWritedBytes < strContent.length())
	{
		DWORD dwWrite = 0;
		//向命名管道中写入数据
		OVERLAPPED osWrite = {0, 0, 0};
		osWrite.hEvent = CreateEvent(FALSE, NULL, NULL, FALSE);

		BOOL errorFlag = FALSE;
		{
			SLOG_INFO<< "before write to pipe, content is " << strContent;
			SLOG_INFO<< "content size is " << strContent.length();

			errorFlag = WriteFile(g_CurPipe, (LPCVOID)(strContent.c_str() + dwCurWritedBytes), 
				strContent.length() - dwCurWritedBytes, &dwWrite, &osWrite);
		}

		if (FALSE == errorFlag)
		{
			int errorCode = GetLastError(); 
			SLOG_INFO<< "send to sync error" << errorCode;

			WaitForSingleObject(osWrite.hEvent, INFINITE);
			result = false;
		}
		else
		{
			dwCurWritedBytes += dwWrite;
			SLOG_INFO<<"send to sync success, write bytes is "<<dwWrite;
		}

		CloseHandle(osWrite.hEvent);

		if (!result)
		{
			break;
		}

	}

	return result;
}


VOID GetAnswerToRequest( char* pchRequest, DWORD cbBytesRead)
{
	std::string request = std::string(pchRequest, cbBytesRead);
	// 解析request，根据不同的request生成不同的response logic
	SLOG_INFO << "pipe request: " << request;
	int type = RequestJsonOperator::GetRequestType(request);
	switch (type)
	{
	case kInitRequest:
		{
			SLOG_INFO<<"monitor init status" << g_InitFlag;
			break;
		}
	case kUpdatePaths:
		{
			RequestJsonOperator::UpdatePaths(request);

			//MonitorManager& monitorManager = MonitorManager::GetInstance();
			//monitorManager.Init(monitorInfo);
			//monitorManager.InitLastUSN(lastUSNInfo);

			//monitorManager.RunAllMonitor();
			break;
		}
	case kUpdateWhiteProcessList:
		{
			RequestJsonOperator::UpdateWhiteProcessList(request);
			break;
		}
	case kUpdateOfflineStrategy:
		// 首先解析时间戳信息，然后将时间戳之后的本地监控数据库记录通过pipe发送到sync端
	{
		SLOG_INFO << "send missing records from select timestamp";

		break;
	}
	default: // for error request type
		break;
	}
	
}
