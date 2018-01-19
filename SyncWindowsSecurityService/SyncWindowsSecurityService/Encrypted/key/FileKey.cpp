#include "FileKey.h"
#include "../../Logger/BoostLog.h"
#include "../../Util/Utils.h"
#include "../encrypter.h"
#include "../md5sums.h"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <fstream>
#include <ShlObj.h>

const string pipe_name = "\\\\.\\pipe\\keypipename";
const string key_file_name = "\\encrypted";
const string bak_key_file_name = "\\encrypted_bak";

using namespace boost;

FileKey::FileKey()
{
	pipe_.reset(new WinNamedPipe);
	if (!pipe_->RunSync(pipe_name))
	{
		SLOG_ERROR << "Key Pipe Run failed";
	}
}


FileKey::KeyBlock FileKey::GetKeyByPipe()
{
	auto ret = pipe_->SendWithReply("file_id");

	assert(ret.size() == 2 * MAX_KEY_LENGTH);

	return KeyBlock{
		ret.substr(0, MAX_KEY_LENGTH),
		ret.substr(MAX_KEY_LENGTH, MAX_KEY_LENGTH * 2)
	};
}


bool FileKey::InitKey(const string& file_cipher, const string& bak_file_cipher)
{
	file_cipher_ = file_cipher_;
	bak_file_cipher_ = bak_file_cipher;

	if (GetKeyFromLocal())
	{
		return true;
	}

	if (!GenerateKey())
	{
		return false;
	}

	return SetKeyForLocal();
}

FileKey::KeyBlock FileKey::GetKey(const string& file_id)
{
#ifdef SINGLE_TEST
	return KeyBlock{ "1111111111111111", "1111111111111111"};
#else	
	return key_block_;
#endif
}

bool FileKey::GetKeyFromLocal()
{
	if (GetKeyFromFile())
	{
		return true;
	}

	if (GetKeyFromFileBak())
	{
		return true;
	}

	return false;
}


bool FileKey::SetKeyForLocal()
{
	if (!SetKeyForFile())
	{
		return false;
	}

	if (!SetKeyForFileBak())
	{
		return false;
	}

	return true;
}

bool FileKey::GetKeyFromFile()
{
	auto path = Utils::GetExePath();
	path.append(key_file_name);

	ifstream file(path.c_str(), ios::in | ios::binary | ios::_Nocreate);
	if (file.is_open())
	{
		char payload[96] = {0};
		file.read(payload, 96);

		FILE_KEY key;
		Encrypter::Decrypt((unsigned char*)payload, 96, (unsigned char*)&key, file_cipher_);

		key_ = key.file_key;

		return true;
	}

	return false;
}

bool FileKey::GetKeyFromFileBak()
{
	auto path = Utils::GetExePath();
	path.append(bak_key_file_name);

	ifstream file(path.c_str(), ios::in | ios::binary | ios::_Nocreate);
	if (file.is_open())
	{
		char payload[96] = { 0 };
		file.read(payload, 96);

		BAK_FILE_KEY key;
		Encrypter::Decrypt((unsigned char*)payload, 96, (unsigned char*)&key, bak_file_cipher_);

		key_ = key.file_key;

		return true;
	}

	return false;
}

bool FileKey::SetKeyForFile()
{
	auto path = Utils::GetExePath();
	path.append(key_file_name);

	ofstream file(path.c_str(), ios::out | ios::binary );
	if (file.is_open())
	{
		FILE_KEY key;
		memcpy(key.file_key, key_.c_str(), AES_KEY_LEN);
		memcpy(key.key_iv, key_.c_str(), AES_KEY_LEN);
		Md5sums::Md5((char*)&key, sizeof(FILE_KEY) - sizeof(key.checksum), key.checksum);

		char payload[96] = { 0 };
		Encrypter::Encrypt((unsigned char*)&key, 96, (unsigned char*)payload, file_cipher_);
		file.write(payload, 96);

		return true;
	}

	return false;
}

bool FileKey::SetKeyForFileBak()
{
	auto path = Utils::GetExePath();
	path.append(bak_key_file_name);

	ofstream file(path.c_str(), ios::out | ios::binary);
	if (file.is_open())
	{
		BAK_FILE_KEY key;
		memcpy(key.file_key, key_.c_str(), AES_KEY_LEN);
		memcpy(key.key_iv, key_.c_str(), AES_KEY_LEN);
		Md5sums::Md5((char*)&key, sizeof(FILE_KEY) - sizeof(key.checksum), key.checksum);

		char payload[96] = { 0 };
		Encrypter::Encrypt((unsigned char*)&key, 96, (unsigned char*)payload, bak_file_cipher_);
		file.write(payload, 96);

		return true;
	}

	return false;
}

bool FileKey::GenerateKey()
{
	key_ = Utils::UniqueString(AES_KEY_LEN);
	return key_ != "";
}

