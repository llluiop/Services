#ifndef __CIPHER_KEY_H__
#define __CIPHER_KEY_H__

#define MAX_KEY_LENGTH 16

#include <string>
#include <functional>
#include "../../driver/msg.h"

using namespace std;

class Cipher
{
public:	
	static Cipher* Instance()
	{
		static Cipher cipher;
		return &cipher;
	}

	bool InitCipher();

	string GetFileCipher()
	{
		return file_cipher_;
	}

	string GetBakFileCiper()
	{
		return bak_file_cipher_;
	}

private:
	Cipher();

	bool GetFileCipherFromLocalFile();
	bool GetBakFileCipherFromLocalFile();

	bool GetFileCipherFromRegister();
	bool GetBakFileCipherFromRegister();

	bool SetFileCipherForLocalFile();
	bool SetBakFileCipherForLocalFile();

	bool SetFileCipherForRegister();
	bool SetBakFileCipherForRegister();

	bool GenerateFileCipher();
	bool GenerateBakFileCipher();


private:
	string file_cipher_; //used for encrypt file_key
	string bak_file_cipher_; //used for encrypt bak_file_key

};

#endif
