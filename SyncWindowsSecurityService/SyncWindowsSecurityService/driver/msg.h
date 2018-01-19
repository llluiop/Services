#ifndef	__MSG_H__
#define __MSG_H__

#include <Windows.h>
#include <tchar.h>
#include <fltUser.h>

#define SCANNER_READ_BUFFER_SIZE   1024
#define IMAGE_NAME_LEN 32
#define MONITOR_PATH_LEN 256
#define MD5_SUM_LEN 16
#define AES_KEY_LEN 16
#define IMAGE_VERSION_LEN 32
#define MAX_KEY_LENGTH 16

#pragma warning(push)
#pragma warning(disable: 4200)



typedef enum _MESSAGE_TYPE
{
	MSG_WHITELIST = 0,
	MSG_ENCRYPTDIR = 1,
	MSG_ENCRYPTKEY = 2,
	MSG_HEARTBEAT = 3,
	MSG_RESPONSE = 4,
	MSG_CHECK_PROCESS_REQ = 5,
	MSG_CHECK_PROCESS_RSP = 6,
	MSG_GET_KEY_REQ = 7,
	MSG_GET_KEY_RSP = 8,
	MSG_NOTIFY = 9,
	MSG_END
} MESSAGE_TYPE;

typedef enum _MESSAGE_OPERATION
{
	OP_ADD = 0,
	OP_DEL = 1,
	OP_QUERY = 2,
	OP_MODIFY = 3,
	OP_NONE = 4,
	OP_END
} MESSAGE_OPERATION;

typedef struct _MESSAGE_HEADER
{
	UINT32 ulMessageID;
	UINT32 ulOperation;
} MESSAGE_HEADER, *PMESSAGE_HEADER;



/*
* the secure path info, which driver just encrypt file in
* this path.
*/
typedef struct _ENCRYPTION_PATH_INFO
{
	wchar_t path[MONITOR_PATH_LEN];

	_ENCRYPTION_PATH_INFO()
	{
		memset(this, 0, sizeof(_ENCRYPTION_PATH_INFO));
	}
} ENCRYPTION_PATH_INFO, *PENCRYPTION_PATH_INFO;

/*
* fangyun client add/query/modify/delete the secure paths info
* to driver. the count means that this message contain how many
* path. maybe it is always 1.
*/
typedef struct _ENCRYPTION_PATH_LIST_INFO
{
	MESSAGE_HEADER Header;
	UINT32 ulCount;
	UINT32 ulReverse;
	ENCRYPTION_PATH_INFO SecuretPaths[];
}ENCRYPTION_PATH_LIST_INFO, *PENCRYPTION_PATH_LIST_INFO;

/*
* encrypt file's key info
*/
typedef struct _ENCRYPTION_KEY_INFO
{
	UCHAR   ucEncryptionKey[AES_KEY_LEN];

	_ENCRYPTION_KEY_INFO()
	{
		memset(this, 0, sizeof(_ENCRYPTION_KEY_INFO));
	}
}ENCRYPTION_KEY_INFO, *PENCRYPTION_KEY_INFO;

/*
* fangyun client add/query/modify/delete the file's key info
* to driver.
*/
typedef struct _KEYS_INFO
{
	MESSAGE_HEADER Header;
	UINT32 ulCount;
	UINT32 ulReverse;
	ENCRYPTION_KEY_INFO Keys[];
}KEYS_INFO, *PKEYS_INFO;

/*
* heart beat between client and driver. client send this
* message every 3(some else?) second, when lost 3(maybe more?)
* heart beats, driver unload it self.
*/
typedef struct _HEART_BEAT
{
	MESSAGE_HEADER Header;
}HEART_BEAT;

/*
* driver send response to client
*/
typedef struct _DRIVER_RESPONESE
{
	UINT32 ulMessageID;
	UINT32 ulResult;
}DRIVER_RESPONESE, *PDRIVER_RESPONESE;



typedef enum _MESSAGE_OPERATION_RESULT {
	OP_RESULT_SUCCESS = 0,
	OP_RESULT_FAILED = 1,
	OP_RESULT_TIMEOUT = 2,
	OP_RESULT_END
}MESSAGE_OPERATION_RESULT;


// 0: black process; 1: white process;
typedef enum _Process_Type {
	BLACK_PROC = 0,
	WHITE_PROC = 1,
	SYSTEM_PROC = 2,
	EXPLORER_PORC = 3,
	OTHER_PROC = 4,
	EXCEL_PROC = 5,
	PPT_PROC = 6,
	FANG_PROC = 7,
	PROC_TYPE_END
}FANGYUN_PROCESS_TYPE;

typedef enum _NOTIFY_OPERATER_TYPE {
	OPERATE_OPEN = 0,
	OPERATE_READ = 1,
	OPERATE_WRITE = 2,
	OPERATE_DELETE = 3,
	OPERATE_CREATE = 4,
	OPERATE_CLOSE = 5,
	OPERATE_COPY = 6,
	OPERATE_MOVE_OUT = 7,
	OPERATE_MOVE_IN = 8,
	OPERATE_RENAME_PATH = 9,
	OPERATE_END
}NOTIFY_OPERATER_TYP;

typedef struct _DRIVER_NOTIFICATION {
	UINT32 hProcessID;
	UINT32 uiOpType;
	CHAR   szTimeStamp[32];
	wchar_t  wsFilePath[MAX_PATH];
}DRIVER_NOTIFICATION, *PDRIVER_NOTIFICATION;

typedef struct _NOTIFY_REQ {
	MESSAGE_HEADER Header;
	DRIVER_NOTIFICATION Param;
}NOTIFY_REQ, *PNOTIFY_REQ;

/*
the request param check process is in white list from driver
*/
typedef struct _CHECK_PROCESS_REQ_PARAM
{
	UINT32 uiProcessID;
	UINT32 lSequenceID;
	UINT8 llTimeStamp[8];
}CHECK_PROCESS_REQ_PARAM, *PCHECK_PROCESS_REQ_PARAM;

/*
the request message check process is in white list from driver
*/
typedef struct _CHECK_PROCESS_REQ
{
	MESSAGE_HEADER Header;
	CHECK_PROCESS_REQ_PARAM Param;
}CHECK_PROCESS_REQ, *PCHECK_PROCESS_REQ;

/*
the result of check process is in white list from user mode
*/
typedef struct _CHECK_PROCESS_RESULT
{
	UINT32 uiResult;
	UINT32 uiProcType;
	UINT32 lSequenceID;
	UINT8 llTimeStamp[8];
}CHECK_PROCESS_RESULT, *PCHECK_PROCESS_RESULT;

/*
the response message check process is in white list from user
*/
typedef struct _CHECK_PROCESS_RSP
{
	MESSAGE_HEADER Header;
	CHECK_PROCESS_RESULT Result;

	_CHECK_PROCESS_RSP()
	{
		Header.ulMessageID = MSG_CHECK_PROCESS_RSP;
	}
}CHECK_PROCESS_RSP, *PCHECK_PROCESS_RSP;

/*
the request param for get key
*/
typedef struct _GET_KEY_PARAM
{
	UINT32 lSequenceID;
	UINT8 llTimeStamp[8];
	UINT8 llFileID[MAX_KEY_LENGTH];
	WCHAR ucFileName[MAX_PATH];
}GET_KEY_PARAM;

/*
the result of get key from user mode
*/
typedef struct _GET_KEY_RESULT
{
	UINT32 uiResult;
	UINT32 lSequenceID;
	UINT8 llTimeStamp[8];
	UINT8 llFileID[MAX_KEY_LENGTH];
	UCHAR ucKey[MAX_KEY_LENGTH];
	UCHAR ucIV[MAX_KEY_LENGTH];

	_GET_KEY_RESULT()
	{
		memset(this, 0, sizeof(_GET_KEY_RESULT));
	}
}GET_KEY_RESULT, *PGET_KEY_RESULT;

/*
the request message for get key
*/
typedef struct _GET_KEY_REQ
{
	MESSAGE_HEADER Header;
	GET_KEY_PARAM Param;
}GET_KEY_REQ, *PGET_KEY_REQ;

/*
the response message for get key
*/
typedef struct _GET_KEY_RSP
{
	MESSAGE_HEADER Header;
	GET_KEY_RESULT Result;

	_GET_KEY_RSP()
	{
		Header.ulMessageID = MSG_GET_KEY_RSP;
	}
}GET_KEY_RSP, *PGET_KEY_RSP;

//////////////////////////////////////////////////////////
typedef struct _FILTER_NOTIFICATION_MESSAGE {
	FILTER_MESSAGE_HEADER MessageHeader;
	NOTIFY_REQ notification;
	OVERLAPPED Ovlp;

} FILTER_NOTIFICATION_MESSAGE, *PFILTER_NOTIFICATION_MESSAGE;

typedef struct _FILTER_PROCESS_CHECK_MESSAGE {
	FILTER_MESSAGE_HEADER MessageHeader;
	CHECK_PROCESS_REQ req;
	OVERLAPPED Ovlp;

} FILTER_PROCESS_CHECK_MESSAGE, *PFILTER_PROCESS_CHECK_MESSAGE;

typedef struct _FILTER_GET_KEY_MESSAGE {
	FILTER_MESSAGE_HEADER MessageHeader;
	GET_KEY_REQ req;
	OVERLAPPED Ovlp;

} FILTER_GET_KEY_MESSAGE, *PFILTER_GET_KEY_MESSAGE;
/////////////////////////////////////////////////////////
typedef struct _FILTER_PROCESS_CHECK_REPLY {
	FILTER_REPLY_HEADER MessageHeader;
	CHECK_PROCESS_RSP req;

} FILTER_PROCESS_CHECK_REPLY, *PFILTER_PROCESS_CHECK_REPLY;

typedef struct _FILTER_GET_KEY_REPLY {
	FILTER_REPLY_HEADER MessageHeader;
	GET_KEY_RSP req;


} FILTER_GET_KEY_REPLY, *PFILTER_GET_KEY_REPLY;
#pragma warning(pop)


#endif
