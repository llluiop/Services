#ifndef __msg_handler_h__
#define __msg_hanlder_h__


#include "msg.h"
#include <string>

using namespace std;

class MsgHandler
{
public:
	static MsgHandler* GetInstance();

public:
	FANGYUN_PROCESS_TYPE GetProcessType(const wstring& name);


private:
	MsgHandler(){}
};

#endif