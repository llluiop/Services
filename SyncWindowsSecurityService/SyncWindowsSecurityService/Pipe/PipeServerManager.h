#ifndef PipeServerManager_h__
#define PipeServerManager_h__


#include <string>
#include <boost/noncopyable.hpp> 
#include <boost/shared_ptr.hpp>
#include "WinNamedPipeServer.h"


class PipeServerManager{
public:
	static PipeServerManager& GetInstance();
	bool Send(const std::string& data);
	void Run();

private:
	PipeServerManager();

public:
	static PipeServerManager *instance_;

private:
	boost::shared_ptr<WinNamedPipeServer> pipe_;

};


#endif // PipeServerManager_h__
