#include "MsgHandler.h"
#include "../Util/Utils.h"
#include "../driver/Strategy.h"

MsgHandler * MsgHandler::GetInstance()
{
	static MsgHandler instance;
	return &instance;
}

FANGYUN_PROCESS_TYPE MsgHandler::GetProcessType(const wstring& name)
{
	if (Utils::IsExplorer(name))
	{
		return EXPLORER_PORC;
	}

	else if (Utils::IsFangCloud(name))
	{
		return FANG_PROC;
	}
	else if (Utils::IsSysProcessWhite(name))
	{
		return SYSTEM_PROC;
	}
	else if (SecurityStrategy::Instance()->IsWhiteProcess(name))
	{
		return WHITE_PROC;
	}
	else
	{
		return BLACK_PROC;
	}

}

