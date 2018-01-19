#ifndef __ENCRYPTER_H__
#define __ENCRYPTER_H__

#include "botan/botan_all.h"
#include <string>

using namespace Botan;
using namespace std;


class Encrypter
{

public:
	static void Encrypt(unsigned char* value, int len, unsigned char* out, const string& passphrase);
	static void Decrypt(unsigned char* payload, int len, unsigned char* out, const string& passphrase);

};

#endif
