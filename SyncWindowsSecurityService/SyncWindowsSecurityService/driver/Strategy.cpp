#include "strategy.h"
#include "../Encrypted/md5sums.h"
#include <tchar.h>
#include <future>

SecurityStrategy * SecurityStrategy::Instance()
{
	static SecurityStrategy instance;
	return &instance;
}

bool SecurityStrategy::SetSecurityPaths(const std::vector<std::string>& monitorPaths)
{
	if (!svr_)
		return false;

	int path_size = monitorPaths.size();

	int size = sizeof(ENCRYPTION_PATH_LIST_INFO) + sizeof(ENCRYPTION_PATH_INFO) * path_size;

	auto deleter = [](PENCRYPTION_PATH_LIST_INFO ptr) {free(ptr); };
	unique_ptr < ENCRYPTION_PATH_LIST_INFO, decltype(deleter) > infos((PENCRYPTION_PATH_LIST_INFO)malloc(size), deleter);
	infos->Header.ulMessageID = MSG_ENCRYPTDIR;
	infos->Header.ulOperation = OP_ADD;
	infos->ulCount = path_size;

	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	for (int i = 0; i < path_size; i++)
	{
		ENCRYPTION_PATH_INFO info;

		std::wstring path = conv.from_bytes(monitorPaths[i]);
		_tcscpy_s(info.path, MONITOR_PATH_LEN, path.c_str());
		memcpy(&infos->SecuretPaths[i], &info, sizeof(ENCRYPTION_PATH_INFO));

		SLOG_INFO << boost::format("ENCRYPTION_PATH_INFO path=%1%") % monitorPaths[i];
	}

	SLOG_INFO << "send ENCRYPTION_PATH_LIST_INFO";
	return svr_->SendSecurityPath(infos.get(), size);
}



bool SecurityStrategy::SetEncryptedKey()
{
	//if (!svr_)
	//	return false;

	//int key_size = 1;

	//int size = sizeof(KEYS_INFO) + sizeof(ENCRYPTION_KEY_INFO) * key_size;

	//auto deleter = [](PKEYS_INFO ptr) {free(ptr); };
	//unique_ptr<KEYS_INFO, decltype(deleter)> infos((PKEYS_INFO)malloc(size), deleter);
	//infos->Header.ulMessageID = MSG_ENCRYPTKEY;
	//infos->Header.ulOperation = OP_ADD;
	//infos->ulCount = key_size;

	//std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//for (int i = 0; i < key_size; i++)
	//{
	//	ENCRYPTION_KEY_INFO info;
	//	char out[AES_KEY_LEN] = { 0 };
	//	key_.Key(out);
	//	memcpy(info.ucEncryptionKey, out, AES_KEY_LEN);


	//	memcpy(&infos->Keys[i], &info, sizeof(ENCRYPTION_KEY_INFO));

	//	SLOG_INFO << boost::format("ENCRYPTION_KEY_INFO key=%s ") % out;

	//}

	//SLOG_INFO << "send WHITE_LIST_INFO";
	//return svr_->Send(infos.get(), size);
	return true;
}

void SecurityStrategy::Filter(const std::shared_ptr<MinifilterSvr>& svr)
{
	svr_ = svr;

	SLOG_INFO << "SecurityStrategy start";

}
