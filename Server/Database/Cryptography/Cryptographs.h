#pragma once
class Cryptography
{
public:
	std::string DecryptAES(const std::string& encryptedtext, const std::string& key);
	std::string EncryptAES(const std::string& plaintext, const std::string& key);
	std::string GenerateAES256Key();
	std::string Base64Encode(const std::string& input);
	std::string Base64Decode(const std::string& input);
};