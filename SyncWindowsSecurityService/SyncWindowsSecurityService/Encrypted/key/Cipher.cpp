#include "Cipher.h"
#include "../../Logger/BoostLog.h"
#include "../../Util/Utils.h"
#include "../encrypter.h"
#include "../md5sums.h"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <fstream>
#include <ShlObj.h>

const std::string cipher = "yifangyunservice";
const string cipher_file_name = "\\cipher";
const string cipher_bak_file_name = "\\bak_cipher";

const LPCTSTR data_set = _T("Software\\FangCloud");

Cipher::Cipher()
{

}

bool Cipher::InitCipher()
{
	if (!GetFileCipherFromLocalFile())
	{
		SLOG_ERROR << "local file cipher failed";

		if (!GetFileCipherFromRegister())
		{
			SLOG_ERROR << "register cipher failed";

			if (!GenerateFileCipher())
			{
				SLOG_ERROR << "generate cipher failed";
				return false;
			}

			SetFileCipherForLocalFile();
			SetFileCipherForRegister();
		}
		else
		{
			SetFileCipherForLocalFile();
		}
	}


	if (!GetBakFileCipherFromLocalFile())
	{
		SLOG_ERROR << "bak local file cipher failed";
		if (!GetBakFileCipherFromRegister())
		{
			SLOG_ERROR << "bak register cipher failed";
			if (!GenerateBakFileCipher())
			{
				SLOG_ERROR << "generate cipher failed";
				return false;
			}
			else
			{
				SetBakFileCipherForLocalFile();
				SetBakFileCipherForRegister();
			}
		}
		else
		{
			SetBakFileCipherForLocalFile();
		}

	}

	return true;
}



bool Cipher::GetFileCipherFromLocalFile()
{
	auto path = Utils::GetExePath();
	path.append(cipher_file_name);

	ifstream file(path.c_str(), ios::in | ios::binary | ios::_Nocreate);
	if (file.is_open())
	{
		char payload[AES_KEY_LEN] = { 0 };
		file.read(payload, AES_KEY_LEN);

		char file_cipher[AES_KEY_LEN] = {0};
		Encrypter::Decrypt((unsigned char*)payload, AES_KEY_LEN, (unsigned char*)file_cipher, cipher);

		file_cipher_ = string(file_cipher, AES_KEY_LEN);

		return true;
	}

	return false;
}

bool Cipher::GetBakFileCipherFromLocalFile()
{
	auto path = Utils::GetExePath();
	path.append(cipher_bak_file_name);

	ifstream file(path.c_str(), ios::in | ios::binary | ios::_Nocreate);
	if (file.is_open())
	{
		char payload[AES_KEY_LEN] = { 0 };
		file.read(payload, AES_KEY_LEN);

		char bak_file_cipher[AES_KEY_LEN] = { 0 };
		Encrypter::Decrypt((unsigned char*)payload, AES_KEY_LEN, (unsigned char*)bak_file_cipher, cipher);

		bak_file_cipher_ = string(bak_file_cipher, AES_KEY_LEN);

		return true;
	}

	return false;
}

bool Cipher::GetFileCipherFromRegister()
{
	HKEY hkey;

	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CURRENT_USER, data_set, 0, KEY_READ, &hkey))
	{
		char dwValue[16];
		DWORD dwSzType = REG_SZ;
		DWORD dwSize = sizeof(dwValue);
		if (::RegQueryValueEx(hkey, _T("file_cipher"), 0, &dwSzType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
		{
			return false;
		}

		char file_cipher[16] = { 0 };
		Encrypter::Decrypt((unsigned char*)dwValue, 32, (unsigned char*)file_cipher, cipher);

		file_cipher_ = string(file_cipher, 16);
	}
	return ERROR_SUCCESS == ::RegCloseKey(hkey);
}

bool Cipher::GetBakFileCipherFromRegister()
{
	HKEY hkey;

	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CURRENT_USER, data_set, 0, KEY_READ, &hkey))
	{
		char dwValue[16];
		DWORD dwSzType = REG_SZ;
		DWORD dwSize = sizeof(dwValue);
		if (::RegQueryValueEx(hkey, _T("bak_file_cipher"), 0, &dwSzType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
		{
			return false;
		}

		char bak_file_cipher[16] = { 0 };
		Encrypter::Decrypt((unsigned char*)dwValue, 32, (unsigned char*)bak_file_cipher, cipher);

		bak_file_cipher_ = string(bak_file_cipher, 16);
	}
	return ERROR_SUCCESS == ::RegCloseKey(hkey);
}

bool Cipher::SetFileCipherForLocalFile()
{
	auto path = Utils::GetExePath();
	path.append(cipher_file_name);

	ofstream file(path.c_str(), ios::out | ios::binary);
	if (file.is_open())
	{
		char payload[AES_KEY_LEN] = { 0 };
		Encrypter::Encrypt((unsigned char*)file_cipher_.c_str(), AES_KEY_LEN, (unsigned char*)payload, cipher);
		file.write(payload, AES_KEY_LEN);

		return true;
	}

	return false;
}

bool Cipher::SetBakFileCipherForLocalFile()
{
	auto path = Utils::GetExePath();
	path.append(cipher_bak_file_name);

	ofstream file(path.c_str(), ios::out | ios::binary);
	if (file.is_open())
	{
		char payload[AES_KEY_LEN] = { 0 };
		Encrypter::Encrypt((unsigned char*)bak_file_cipher_.c_str(), AES_KEY_LEN, (unsigned char*)payload, cipher);
		file.write(payload, AES_KEY_LEN);

		return true;
	}

	return false;
}

bool Cipher::SetFileCipherForRegister()
{
	HKEY hkey;
	DWORD dis;
	if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_CURRENT_USER, data_set, 0, 0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hkey, &dis))
	{
		char payload[AES_KEY_LEN] = { 0 };
		Encrypter::Encrypt((unsigned char*)file_cipher_.c_str(), AES_KEY_LEN, (unsigned char*)payload, cipher);
		if (ERROR_SUCCESS != ::RegSetValueEx(hkey, _T("file_cipher"), 0, REG_BINARY, (unsigned char *)payload, AES_KEY_LEN))
		{
			return false;
		}
		
	}
	return ERROR_SUCCESS != ::RegCloseKey(hkey);
}

bool Cipher::SetBakFileCipherForRegister()
{
	HKEY hkey;
	DWORD dis;
	if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_CURRENT_USER, data_set, 0, 0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hkey, &dis))
	{
		char payload[AES_KEY_LEN] = { 0 };
		Encrypter::Encrypt((unsigned char*)bak_file_cipher_.c_str(), AES_KEY_LEN, (unsigned char*)payload, cipher);
		if (ERROR_SUCCESS != ::RegSetValueEx(hkey, _T("bak_file_cipher"), 0, REG_BINARY, (unsigned char *)payload, AES_KEY_LEN))
		{
			return false;
		}

	}
	return ERROR_SUCCESS != ::RegCloseKey(hkey);
}

bool Cipher::GenerateFileCipher()
{
	file_cipher_ = Utils::UniqueString(AES_KEY_LEN);
	return file_cipher_ != "";
}

bool Cipher::GenerateBakFileCipher()
{
	bak_file_cipher_ = Utils::UniqueString(AES_KEY_LEN);
	return bak_file_cipher_ != "";
}

