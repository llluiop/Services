#include "WindowsServiceEventControl.h"


ServiceStopEvent *ServiceStopEvent::instance_ = NULL;


ServiceStopEvent& ServiceStopEvent::GetInstance()
{
	if (NULL == instance_)
		instance_ = new ServiceStopEvent;

	return *instance_;
}


bool ServiceStopEvent::Create()
{
	serviceStopEvent_ = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (NULL == serviceStopEvent_) 
	{
		return false;
	}

	return true;
}


void ServiceStopEvent::Close()
{
	CloseHandle (serviceStopEvent_);
}


void ServiceStopEvent::SetHappen()
{
	 SetEvent (serviceStopEvent_);
}


bool ServiceStopEvent::IsHappen()
{
	return WaitForSingleObject(serviceStopEvent_, 0) == WAIT_OBJECT_0;
}


ServiceStopEvent::ServiceStopEvent() : serviceStopEvent_(INVALID_HANDLE_VALUE)
{
	// do something

}

