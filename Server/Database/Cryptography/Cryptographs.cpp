#include "Pch.h"
#include "Cryptographs.h"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#pragma comment(lib, "libcrypto.lib")
std::string Cryptography::GenerateAES256Key()
{
	unsigned char key[32];
	if (RAND_bytes(key, sizeof(key)) != 1)
	{
		std::cerr << "Error generating a random key" << std::endl;
		return "";
	}

	std::string keystring(reinterpret_cast<const char*>(key), sizeof(key));
	return keystring;
}
std::string Cryptography::Base64Encode(const std::string& input)
{
	BIO* bio;
	BIO* b64;
	BUF_MEM* buffer;

	bio = BIO_new(BIO_s_mem());
	b64 = BIO_new(BIO_f_base64());
	BIO_push(b64, bio);

	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(b64, input.c_str(), input.length());
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &buffer);

	std::string encoded(buffer->data, buffer->length);

	BIO_free_all(b64);

	return encoded;
}

std::string Cryptography::Base64Decode(const std::string& input)
{
	BIO* bio;
	BIO* b64;
	char* buffer = new char[input.length()];
	BIO* inputbio = BIO_new_mem_buf(input.c_str(), input.length());

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	BIO_push(b64, bio);

	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO_set_flags(inputbio, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(bio, input.c_str(), input.length());
	BIO_flush(bio);

	int decodedLen = BIO_read(b64, buffer, input.length());

	BIO_free_all(b64);
	BIO_free(inputbio);

	std::string decoded(buffer, decodedLen);
	delete[] buffer;

	return decoded;
}
std::string Cryptography::EncryptAES(const std::string& plaintext, const std::string& key)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), NULL) != 1)
	{
		std::cerr << "EVP_EncryptInit_ex failed" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	int outlen = plaintext.length() + AES_BLOCK_SIZE; // Ensure enough space for padding
	unsigned char* encrypted = new unsigned char[outlen];
	int encryptedlen = 0;

	if (EVP_EncryptUpdate(ctx, encrypted, &encryptedlen, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length()) != 1)
	{
		std::cerr << "EVP_EncryptUpdate failed" << std::endl;
		delete[] encrypted;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	int finallen = 0;
	if (EVP_EncryptFinal_ex(ctx, encrypted + encryptedlen, &finallen) != 1) {
		std::cerr << "EVP_EncryptFinal_ex failed" << std::endl;
		delete[] encrypted;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	encryptedlen += finallen;

	std::string encryptedtext(reinterpret_cast<const char*>(encrypted), encryptedlen);
	delete[] encrypted;
	EVP_CIPHER_CTX_free(ctx);

	return encryptedtext;
}

std::string Cryptography::DecryptAES(const std::string& encryptedtext, const std::string& key)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	if (EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), NULL) != 1)
	{
		std::cerr << "EVP_DecryptInit_ex failed" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	int outlen = encryptedtext.length();
	unsigned char* decrypted = new unsigned char[outlen];
	int decryptedlen = 0;

	if (EVP_DecryptUpdate(ctx, decrypted, &decryptedlen, reinterpret_cast<const unsigned char*>(encryptedtext.c_str()), encryptedtext.length()) != 1)
	{
		std::cerr << "EVP_DecryptUpdate failed" << std::endl;
		delete[] decrypted;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	int finallen = 0;
	if (EVP_DecryptFinal_ex(ctx, decrypted + decryptedlen, &finallen) != 1)
	{
		std::cerr << "EVP_DecryptFinal_ex failed" << std::endl;
		delete[] decrypted;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	decryptedlen += finallen;

	std::string decryptedtext(reinterpret_cast<const char*>(decrypted), decryptedlen);
	delete[] decrypted;
	EVP_CIPHER_CTX_free(ctx);

	return decryptedtext;
}