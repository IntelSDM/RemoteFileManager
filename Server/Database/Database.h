#pragma once
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "Cryptographs.h"
enum class CreateUserResult
{
	UserAlreadyExists,
	SerialTooLong,
	HardwareHashTooLong,
	UsernameTooLong,
	PasswordTooLong,
	UsernameTooShort,
	PasswordTooShort,
	Success,
	Injection
};
enum class LoginUserResult
{
	UserDoesNotExist,
	IncorrectPassword,
	InvalidHardware,
	InvalidSerial,
	Banned,
	Success,
	Injection
};


class Database
{
protected:
	sql::mysql::MySQL_Driver* Driver;
	sql::Connection* Connection;
	const std::vector<std::string> DatabaseNames = { "DevBuild" };
	std::wstring Username;
	std::wstring Password;
	bool LoggedIn = false;
	int ActiveDatabase = 0;
	const std::string EncryptionKey = "6cFVnSZ9Kza9aKwDmhT6aVcJ1pXXDSjiUzSAv+9jbT2hYkML92E+g53O+w3SYlhd"; // 32 bytes
private:
	void CreateDatabase();
	void StartDatabase();
	void CreateTables();
	std::string ToLower(const std::string& input);
	std::wstring ToLower(const std::wstring& input);
	sql::SQLString ToSQLString(const std::wstring& input);
	std::wstring SQLStringToWString(const sql::SQLString& sqlstring);
	std::wstring GenerateSalt();
	std::string GenerateKeyName();
	Cryptography Cryptography; // We use a class instance instead of a static namespace as we are multithreading.
public:
	CreateUserResult CreateUser(const std::wstring& username, const std::wstring& password, const std::wstring& cpuname, const std::wstring& gpuname, const std::wstring& motherboard,
		const std::wstring& ramsize, const std::vector<std::string>& driveserials, const std::vector<std::string> ramserials, const std::string moboserial);
	LoginUserResult LoginUser(const std::wstring& username, const std::wstring& password, const std::wstring& cpuname, const std::wstring& gpuname, const std::wstring& motherboard,
		const std::wstring& ramsize, const std::vector<std::string>& driveserials, const std::vector<std::string> ramserials, const std::string moboserial);
	Database();
	~Database();
};