#ifndef PipeManager_h__
#define PipeManager_h__

#include <string>
#include <boost/noncopyable.hpp> 
#include <boost/shared_ptr.hpp>
#include "WinNamedPipe.h"


class PipeManager{
public:
	static PipeManager& GetInstance();
	void Run();
	bool Send(const std::string& data);

private:
	PipeManager();

public:
	static PipeManager *instance_;

private:
	boost::shared_ptr<WinNamedPipe> pipe_;

};


#endif // PipeManager_h__
