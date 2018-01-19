#include "PipeManager.h"


using namespace std;


PipeManager *PipeManager::instance_ = NULL;


PipeManager& PipeManager::GetInstance()
{
	if (NULL == instance_)
		instance_ = new PipeManager;

	return *instance_;
}


PipeManager::PipeManager():pipe_(new WinNamedPipe)
{
}


void PipeManager::Run()
{
	if (pipe_)
	{
		string pipeName = "\\\\.\\pipe\\syncsecuritypipe";
		pipe_->Run(pipeName);
	}
}

bool PipeManager::Send(const string& data)
{
	if (pipe_)
	{
		return pipe_->Send(data);
	}
	else
		return false;
}
