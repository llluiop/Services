#ifndef WindowsServiceEventControl_h__
#define WindowsServiceEventControl_h__

#include <Windows.h>
#include <boost/noncopyable.hpp>


class ServiceStopEvent : boost::noncopyable
{
public:
	static ServiceStopEvent& GetInstance();

	bool Create();
	void Close();
	void SetHappen();
	bool IsHappen();

protected:
	ServiceStopEvent();

public:
	static ServiceStopEvent *instance_;

private:
	HANDLE serviceStopEvent_;

};


#endif // WindowsServiceEventControl_h__
