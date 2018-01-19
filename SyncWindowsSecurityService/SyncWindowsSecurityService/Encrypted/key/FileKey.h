#ifndef __FILE_KEY_H__
#define __FILE_KEY_H__



#include <string>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <mutex>
#include "../../Pipe/WinNamedPipe.h"
#include "../../driver/msg.h"

using namespace std;

class FileKey
{
public:
	struct KeyBlock
	{
		string key;
		string iv;
	};

#pragma pack(push)
#pragma pack(1)
	struct FILE_KEY
    {
		int version;
		int len;
		char reserve[8];
		char file_id[16];
		char file_key[AES_KEY_LEN];
		char key_iv[AES_KEY_LEN];
		char noise[16];
		char checksum[16];

		FILE_KEY()
		{
			memset(this, 0, sizeof(FILE_KEY));
		};
	};

	struct BAK_FILE_KEY
	{
		int version;
		int len;
		char reserve[8];
		char key_iv[16];
		char file_key[AES_KEY_LEN];
		char file_id[AES_KEY_LEN];
		char noise[16];
		char checksum[16];

		BAK_FILE_KEY()
		{
			memset(this, 0, sizeof(BAK_FILE_KEY));
		};
	};
#pragma pack(pop)

	static FileKey* Instance()
	{
		static FileKey key;
		return &key;
	}

	KeyBlock GetKey(const string& fild_id);
	bool InitKey(const string& file_cipher, const string& bak_file_cipher);


private:
	FileKey();
	KeyBlock GetKeyByPipe();
	bool GenerateKey();

	bool GetKeyFromLocal();
	bool SetKeyForLocal();

	bool GetKeyFromFile();
	bool GetKeyFromFileBak();

	bool SetKeyForFile();
	bool SetKeyForFileBak();

private:
	std::shared_ptr<WinNamedPipe> pipe_;
	KeyBlock key_block_;

	std::string file_cipher_; //used for encrypt file_key
	std::string bak_file_cipher_; //used for encrypt bak_file_key
	std::string key_;
};

#endif
