#include "PipeServerManager.h"


using namespace std;


PipeServerManager *PipeServerManager::instance_ = NULL;


PipeServerManager& PipeServerManager::GetInstance()
{
	if (NULL == instance_)
		instance_ = new PipeServerManager;

	return *instance_;
}


PipeServerManager::PipeServerManager():pipe_(new WinNamedPipeServer)
{
}


bool PipeServerManager::Send(const std::string& data)
{
	if (pipe_)
	{
		return pipe_->Send(data);
	}
	else
		return false;
}


void PipeServerManager::Run()
{
	if (pipe_)
	{
		string pipeName = "\\\\.\\pipe\\securityserverpipe";
		pipe_->Run(pipeName);
	}
}
