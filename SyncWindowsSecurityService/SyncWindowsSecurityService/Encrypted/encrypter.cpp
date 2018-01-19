#include "encrypter.h"



void Encrypter::Encrypt(unsigned char* value, int len, unsigned char* out, const string& passphrase)
{
	std::unique_ptr<HashFunction> hash = HashFunction::create("MD5");
	if (!hash)
	{
		return;
	}
	SymmetricKey key = hash->process(passphrase);

	std::unique_ptr<Botan::BlockCipher> cipher(Botan::BlockCipher::create("AES-128"));
	cipher->set_key(key);


	if (len <= cipher->block_size())
	{
		secure_vector<uint8_t> in(value, value + len);
		cipher->encrypt(in.data(), (uint8_t*)out);
	}
	else
	{
		secure_vector<uint8_t> v_in(value, value + len);
		secure_vector<uint8_t> v_out(len);
		cipher->encrypt(v_in, v_out);
		memcpy(out, v_out.data(), len);
	}

}

void Encrypter::Decrypt(unsigned char* payload, int len, unsigned char* out, const string& passphrase)
{
	std::unique_ptr<HashFunction> hash = HashFunction::create("MD5");
	if (!hash)
	{
		return;
	}
	SymmetricKey key = hash->process(passphrase);

	std::unique_ptr<Botan::BlockCipher> cipher(Botan::BlockCipher::create("AES-128"));
	cipher->set_key(key);


	if (len <= cipher->block_size())
	{
		secure_vector<uint8_t> in(payload, payload + len);
		cipher->decrypt(in.data(), (uint8_t*)out);
	}
	else
	{
		secure_vector<uint8_t> v_in(payload, payload + len);
		secure_vector<uint8_t> v_out(len);
		cipher->decrypt(v_in, v_out);
		memcpy(out, v_out.data(), len);
	}




}


