#ifndef RequestJsonOperator_h__
#define RequestJsonOperator_h__


#include <string>
#include "RequestDefine.h"
#include "MonitorDefine.h"


class RequestJsonOperator{
public:
	static int GetRequestType(const std::string& request);


	static bool UpdatePaths(const std::string& request);
	static bool UpdateWhiteProcessList(const std::string& request);


};


#endif // RequestJsonOperator_h__
