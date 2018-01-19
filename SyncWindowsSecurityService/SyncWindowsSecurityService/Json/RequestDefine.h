#ifndef RequestDefine_h__
#define RequestDefine_h__


const int kErrorRequest = -1;
const int kInitRequest = 0;
const int kUpdatePaths = 1;
const int kUpdateWhiteProcessList = 2;
const int kUpdateOfflineStrategy = 3;


class Request
{
public:
	void SetRequestType(int type){type_ = type;}
	int RequestType() const{return type_;}

public:
	int type_;
};


class UpdatePaths : public Request
{
public:
	UpdatePaths()
	{
		SetRequestType(kUpdatePaths);
	}
};


class UpdateWhiteProcessList : public Request
{
public:
	UpdateWhiteProcessList()
	{
		SetRequestType(kUpdateWhiteProcessList);
	}
};





#endif // RequestDefine_h__
