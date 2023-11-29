#include "Pch.h"
#include "Database.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/sqlstring.h>
#pragma comment(lib, "mysqlcppconn.lib")
#include "SHA256.h"


Database::Database()
{
	Database::StartDatabase();
	Database::CreateDatabase();
	Database::CreateTables();
}
std::string Database::ToLower(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
		{
			return std::tolower(c);
		});
	return result;
}
std::wstring Database::ToLower(const std::wstring& input)
{
	std::wstring result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
		{
			return std::tolower(c);
		});
	return result;
}
sql::SQLString Database::ToSQLString(const std::wstring& input)
{
	std::string utf8(input.begin(), input.end());
	return sql::SQLString(utf8.c_str());
}
std::wstring Database::SQLStringToWString(const sql::SQLString& sqlstring)
{
	const std::string utf8str = sqlstring.asStdString();
	std::wstring wstr;

	for (char ch : utf8str)
	{
		wstr += static_cast<wchar_t>(ch);
	}

	return wstr;
}

std::wstring Database::GenerateSalt()
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const int length = 16;
	srand(__rdtsc());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

	std::wstring salt;
	salt.reserve(length);

	for (int i = 0; i < length; ++i)
	{
		salt += charset[distribution(gen)];
	}

	return salt;
}
std::string Database::GenerateKeyName()
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const int length = 32;
	srand(__rdtsc());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

	std::string key;
	key.reserve(length);

	for (int i = 0; i < length; ++i)
	{
		key += charset[distribution(gen)];
	}

	return key;
}
void Database::CreateDatabase()
{
	sql::ResultSet* result = Connection->createStatement()->executeQuery("SHOW DATABASES");
	std::map<std::string, bool> databaseexistance;

	// set up datavasexistance with our databases
	for (std::string databasename : Database::DatabaseNames)
	{
		databaseexistance[Database::ToLower(databasename)] = false;
	}

	// set all the current databases we got from the query to true
	while (result->next())
	{
		std::string existingdatabasename = result->getString(1);
		databaseexistance[existingdatabasename] = true;
	}

	// check if databases exist
	bool dbresult = false;
	for (auto existancepair : databaseexistance)
	{
		if (existancepair.second == false)
		{
			// database not found, create it!
			std::cout << "Creating " << existancepair.first << std::endl;
			dbresult = Connection->createStatement()->execute("CREATE DATABASE " + existancepair.first);
			if (dbresult)
			{
				std::cout << existancepair.first << " Created Successfully." << std::endl;
			}
			else
			{
				std::cout << "Error Creating " << existancepair.first << std::endl;
			}
		}
		else
		{
			std::cout << existancepair.first << " Found\n";
		}
	}
}
ResetPasswordResult Database::ResetPassword(const std::wstring& username, const std::wstring& newpassword, const std::wstring& cpuname, const std::wstring& gpuname, const std::wstring& motherboard,
	const std::wstring& ramsize, const std::vector<std::string>& driveserials, const std::vector<std::string> ramserials, const std::string moboserial)
{
	if (username.length() > 255)
	{
		return ResetPasswordResult::UsernameTooLong;
	}
	if (newpassword.length() > 255)
	{
		return ResetPasswordResult::PasswordTooLong;
	}
	try
	{
		Connection->setSchema(DatabaseNames[ActiveDatabase]);
		sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM Users WHERE Username = ?");
		statement->setString(1, ToSQLString(ToLower(username)));
		sql::ResultSet* result = statement->executeQuery();
		if (!result->next())
		{
			return ResetPasswordResult::UserDoesNotExist;
		}
		int userid = result->getInt(1);
		delete result;
		delete statement;
		{
			for (std::string serial : ramserials)
			{

				sql::PreparedStatement* statement = Connection->prepareStatement("SELECT RamSerial FROM RamHwids WHERE RamSerial = ? And UserID = ?");
				statement->setString(1, serial);
				statement->setInt(2, userid);
				sql::ResultSet* result = statement->executeQuery();
				if (!result->next())
				{
					return ResetPasswordResult::InvalidSerial;
				}

			}
			for (std::string serial : driveserials)
			{

				sql::PreparedStatement* statement = Connection->prepareStatement("SELECT DriveSerial FROM DiskHwids WHERE DriveSerial = ? And UserID = ?");
				statement->setString(1, serial);
				statement->setInt(2, userid);
				sql::ResultSet* result = statement->executeQuery();
				if (!result->next())
				{
					return ResetPasswordResult::InvalidSerial;
				}

			}
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM MotherboardHwids WHERE MoboSerial = ? And UserID = ?");
				statement->setString(1, moboserial);
				statement->setInt(2, userid);
				sql::ResultSet* result = statement->executeQuery();
				if (!result->next())
				{
					return ResetPasswordResult::InvalidSerial;
				}
			}
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("SELECT ActiveHwid FROM Users WHERE UserID = ?");
				statement->setInt(1, userid);
				sql::ResultSet* result = statement->executeQuery();
				result->next();
				int activehwid = result->getInt(1);

				statement = Connection->prepareStatement("SELECT HWIDHash FROM HwidTable WHERE UserID = ? And HwidInstance = ?");
				statement->setInt(1, userid);
				statement->setInt(2, activehwid);
				result = statement->executeQuery();
				if (!result->next())
				{
					std::wcout << Username + L": Unset HWID" << std::endl;
					ResetPasswordResult::InvalidHardwareHash;
				}
				sql::SQLString hwidhash = result->getString(1);
				if (SQLStringToWString(hwidhash) != sha256(cpuname + gpuname + ramsize + motherboard))
				{
					return ResetPasswordResult::InvalidHardwareHash;
				}
			}
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("UPDATE Users SET Password = ?, Salt = ? WHERE UserID = ?");
				std::wstring salt = GenerateSalt();

				statement->setString(1, ToSQLString(sha256(newpassword + salt)));
				statement->setString(2, ToSQLString(salt));
				statement->setInt(3, userid);
				statement->execute();
			}

		}
		return ResetPasswordResult::Success;

	}
	catch (sql::SQLException ex)
	{
		return ResetPasswordResult::Injection;
	}
}


void Database::CreateTables()
{
	// we do this for all our databases so we can set up a database such as a development database and commercial database with the same tables
	for (std::string database : Database::DatabaseNames)
	{
	Database:Connection->setSchema(database);

		sql::ResultSet* res;
		res = Connection->createStatement()->executeQuery("SHOW TABLES");
		std::map<std::string, bool> usedtables;
		while (res->next())
		{
			std::cout << "Found Table: " << res->getString(1) << std::endl;
			usedtables[Database::ToLower(res->getString(1))] = true;
		}
		// create our tables here
		if (usedtables[Database::ToLower("Users")] == false)
		{
			Connection->createStatement()->execute("CREATE TABLE Users ("
				"UserID INT AUTO_INCREMENT PRIMARY KEY,"
				"Username VARCHAR(255) NOT NULL,"
				"Password VARCHAR(255),"
				"Salt VARCHAR(17))"); 
		}
		if (usedtables[Database::ToLower("EncryptionTable")] == false)
		{
			Connection->createStatement()->execute("CREATE TABLE EncryptionTable ("
				"EncryptionID INT AUTO_INCREMENT PRIMARY KEY,"
				"UserID INT,"
				"Encryption VARCHAR(255),"
				"FOREIGN KEY(UserID) REFERENCES Users(UserID))");
		}
		delete res;
	}
}
void Database::StartDatabase()
{
	Driver = sql::mysql::get_mysql_driver_instance();
	Connection = Driver->connect("tcp://127.0.0.1:3306", "root", "ShoPass8S9Q3L");

}



LoginUserResult Database::LoginUser(const std::wstring& username, const std::wstring& password, const std::wstring& cpuname, const std::wstring& gpuname, const std::wstring& motherboard,
	const std::wstring& ramsize, const std::vector<std::string>& driveserials, const std::vector<std::string> ramserials, const std::string moboserial)
{
	try
	{
		Connection->setSchema(DatabaseNames[ActiveDatabase]);
		int userid = 0;
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();

			if (!result->next())
			{
				std::cout << "User Doesn't Exist" << std::endl;
				return LoginUserResult::UserDoesNotExist;

			}
			userid = result->getInt(1);
			delete result;
			delete statement;
		}
		std::string encryptionkey = "";
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT Encryption FROM EncryptionTable WHERE UserID = ?");
			statement->setInt(1, userid);
			sql::ResultSet* result = statement->executeQuery();
			if (!result->next())
			{
				std::cout << "Encryption Key Doesn't Exist" << std::endl;
				return LoginUserResult::UserDoesNotExist;
			}
			encryptionkey = result->getString(1);
			encryptionkey = Cryptography.DecryptAES(Cryptography.Base64Decode(encryptionkey), EncryptionKey);
			delete result;
			delete statement;

		}
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT Password, Salt FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();
			result->next();
			std::string pass = result->getString(1);
			sql::SQLString salt = result->getString(2);
			delete result;
			if (sha256(password + SQLStringToWString(salt)) != SQLStringToWString(Cryptography.DecryptAES(Cryptography.Base64Decode(pass), encryptionkey)))
			{
				std::cout << "Invalid Password" << std::endl;
				return LoginUserResult::IncorrectPassword;
			}
			delete statement;
		}
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT ActiveHwid FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();
			result->next();
			int activehwid = result->getInt(1);


			statement = Connection->prepareStatement("SELECT HWIDHash FROM HwidTable WHERE UserID = ? And HwidInstance = ?");
			statement->setInt(1, userid);
			statement->setInt(2, activehwid);
			result = statement->executeQuery();
			if (!result->next())
			{
				std::wcout << Username + L": Unset HWID" << std::endl;
			}
			sql::SQLString hwidhash = result->getString(1);
			if (SQLStringToWString(hwidhash) != sha256(cpuname + gpuname + ramsize + motherboard))
			{
				std::cout << "HWID Mismatch" << std::endl;
				return LoginUserResult::InvalidHardware;
			}
			auto now = std::chrono::system_clock::now();
			std::time_t time = std::chrono::system_clock::to_time_t(now);
			std::tm localtime = *std::localtime(&time);
			std::string year = std::to_string(localtime.tm_year + 1900);
			std::string month = std::to_string(localtime.tm_mon + 1);
			std::string day = std::to_string(localtime.tm_mday);
			std::string hour = std::to_string(localtime.tm_hour);
			std::string minute = std::to_string(localtime.tm_min);
			std::string second = std::to_string(localtime.tm_sec);
			std::string timeofcreation = day + "/" + month + "/" + year + " " + hour + ":" + minute;
			for (std::string serial : driveserials)
			{
				if (serial.length() > 255)
				{
					std::cout << "Serial Too Long" << std::endl;
					return LoginUserResult::InvalidSerial;
				}

				statement = Connection->prepareStatement("SELECT DriveSerial FROM DiskHwids WHERE UserID = ? And DriveSerial = ?");
				statement->setInt(1, userid);
				statement->setString(2, serial);
				result = statement->executeQuery();
				if (!result->next())
				{
					std::cout << "Unset Drive HWID Error" << std::endl;
					sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO DiskHwids (UserID, TimeOfCreation, DriveSerial, Banned) VALUES (?, ?, ?, ?)");
					statement->setInt(1, userid);
					statement->setString(2, timeofcreation);
					statement->setString(3, serial);
					statement->setInt(4, 0);
					statement->execute();
				}

			}
			for (std::string serial : ramserials)
			{
				if (serial.length() > 255)
				{
					std::cout << "Serial Too Long" << std::endl;
					return LoginUserResult::InvalidSerial;
				}
				statement = Connection->prepareStatement("SELECT RamSerial FROM RamHwids WHERE UserID = ? And RamSerial = ?");
				statement->setInt(1, userid);
				statement->setString(2, serial);
				result = statement->executeQuery();
				if (!result->next())
				{
					std::cout << "Unset Ram HWID Error" << std::endl;
					sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO RamHwids (UserID, TimeOfCreation, RamSerial, Banned) VALUES (?, ?, ?, ?)");
					statement->setInt(1, userid);
					statement->setString(2, timeofcreation);
					statement->setString(3, serial);
					statement->setInt(4, 0);
					statement->execute();
				}

			}
			if (moboserial.length() > 255)
			{
				std::cout << "Serial Too Long" << std::endl;
				return LoginUserResult::InvalidSerial;
			}
			statement = Connection->prepareStatement("SELECT MoboSerial FROM MotherboardHwids WHERE UserID = ? And MoboSerial = ?");
			statement->setInt(1, userid);
			statement->setString(2, moboserial);
			result = statement->executeQuery();
			if (!result->next())
			{
				std::cout << "Unset Mobo HWID Error" << std::endl;
				statement = Connection->prepareStatement("INSERT INTO MotherboardHwids (UserID, TimeOfCreation, MoboSerial, Banned) VALUES (?, ?, ?, ?)");
				statement->setInt(1, userid);
				statement->setString(2, timeofcreation);
				statement->setString(3, moboserial);
				statement->setInt(4, 0);
				statement->execute();
			}

			{

				statement = Connection->prepareStatement("SELECT Reason FROM BannedUsers WHERE UserID = ?");
				statement->setInt(1, userid);
				result = statement->executeQuery();
				if (result->next())
				{
					std::cout << "User Banned" << std::endl;
					BanUser(username, result->getString(1)); // ban any new data they might have used, new serials on login
					return LoginUserResult::Banned;
				}
				if (BanEvasionCheck(userid, driveserials, ramserials, moboserial)) // add check for product so they cant just test till they spoof the bans
				{
					std::cout << "User Banned For Evasion" << std::endl;
					BanUser(username, "Ban Evasion"); // ban evasion, ban new serials and deny them entry
					return LoginUserResult::Banned;
				}

			}

			delete result;
			delete statement;
		}

		return LoginUserResult::Success;
	}
	catch (sql::SQLException ex)
	{
		return LoginUserResult::Injection;
	}

}

CreateUserResult Database::CreateUser(const std::wstring& username, const std::wstring& password, const std::wstring& cpuname, const std::wstring& gpuname, const std::wstring& motherboard,
	const std::wstring& ramsize, const std::vector<std::string>& driveserials, const std::vector<std::string> ramserials, const std::string moboserial)
{
	try
	{
		Connection->setSchema(DatabaseNames[ActiveDatabase]);

		if (username.length() > 255)
		{
			std::cout << "Username Too Long" << std::endl;
			return CreateUserResult::UsernameTooLong;
		}
		if (username.length() < 3)
		{
			std::cout << "Username Too Long" << std::endl;
			return CreateUserResult::UsernameTooShort;
		}
		// check if user exists
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();

			if (result->next())
			{
				std::cout << "User already exists" << std::endl;
				return CreateUserResult::UserAlreadyExists;
			}
			delete result;
			delete statement;
		}
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm localtime = *std::localtime(&time);
		std::string year = std::to_string(localtime.tm_year + 1900);
		std::string month = std::to_string(localtime.tm_mon + 1);
		std::string day = std::to_string(localtime.tm_mday);
		std::string hour = std::to_string(localtime.tm_hour);
		std::string minute = std::to_string(localtime.tm_min);
		std::string second = std::to_string(localtime.tm_sec);
		std::string timeofcreation = day + "/" + month + "/" + year + " " + hour + ":" + minute;
		if (sha256(cpuname + gpuname + ramsize + motherboard).size() > 255)
		{
			std::cout << "Hardware Hash Too Long" << std::endl;
			return CreateUserResult::HardwareHashTooLong;
		}
		std::wstring salt = Database::GenerateSalt();
		if (password.length() < 5)
		{
			std::cout << "Password Too Short" << std::endl;
			return CreateUserResult::PasswordTooShort;
		}
		if (sha256(password + salt).length() > 255)
		{
			std::cout << "Password Too Long" << std::endl;
			return CreateUserResult::PasswordTooLong;
		}
		if (moboserial.length() > 255)
		{
			std::cout << "Serial Too Long" << std::endl;
			return CreateUserResult::SerialTooLong;
		}
		for (std::string serial : driveserials)
		{
			if (serial.length() > 255)
			{
				std::cout << "Serial Too Long" << std::endl;
				return CreateUserResult::SerialTooLong;
			}
		}
		for (std::string serial : ramserials)
		{
			if (serial.length() > 255)
			{
				std::cout << "Serial Too Long" << std::endl;
				return CreateUserResult::SerialTooLong;
			}
		}

		{
			sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO Users (Username, Password, Salt,ActiveHWID) VALUES (?, ?, ?,?)");
			statement->setString(1, ToSQLString(ToLower(username)));
			statement->setString(2, ToSQLString(sha256(password + salt)));
			statement->setString(3, ToSQLString(salt));
			statement->setInt(4, 0);
			statement->execute();
			delete statement;
		}
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();
			result->next();
			int userid = result->getInt(1);
			delete result;
			std::string encryptionkey = Cryptography.GenerateAES256Key();
			{
				// we know userid cant already be in here because we already check if the username exists or not so no duplicates.
				sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO EncryptionTable (UserID,Encryption) VALUES (?, ?)");
				statement->setInt(1, userid);
				statement->setString(2, Cryptography.Base64Encode(Cryptography.EncryptAES(encryptionkey, EncryptionKey)));
				statement->execute();
				delete statement;
			}
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("Update Users SET Password = ? WHERE UserID = ?");
				std::wstring hashedpass = sha256(password + salt);
				std::string shorthashedpass(hashedpass.begin(), hashedpass.end());
				statement->setString(1, Cryptography.Base64Encode(Cryptography.EncryptAES(shorthashedpass, encryptionkey)));
				statement->setInt(2, userid);
				statement->execute();
				delete statement;
			}
			delete statement;
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO HwidTable (UserID, HwidInstance, CPU, GPU,Motherboard, Ram, HWIDHash,TimeOfCreation) VALUES (?, ?, ?, ?, ?,?, ?,?)");
				statement->setInt(1, userid);
				statement->setInt(2, 0);
				statement->setString(3, ToSQLString(cpuname));
				statement->setString(4, ToSQLString(gpuname));
				statement->setString(5, ToSQLString(motherboard));
				statement->setString(6, ToSQLString(ramsize));
				statement->setString(7, ToSQLString(sha256(cpuname + gpuname + ramsize + motherboard)));
				statement->setString(8, timeofcreation);
				statement->execute();
				delete statement;
			}
			{
				for (std::string serial : driveserials)
				{
					sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO DiskHwids (UserID, TimeOfCreation, DriveSerial, Banned) VALUES (?, ?, ?, ?)");
					statement->setInt(1, userid);
					statement->setString(2, timeofcreation);
					statement->setString(3, serial);
					statement->setInt(4, 0);
					statement->execute();
					delete statement;
				}
			}
			{
				sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO MotherboardHwids (UserID, TimeOfCreation, MoboSerial, Banned) VALUES (?, ?, ?, ?)");
				statement->setInt(1, userid);
				statement->setString(2, timeofcreation);
				statement->setString(3, moboserial);
				statement->setInt(4, 0);
				statement->execute();
				delete statement;
			}
			{
				for (std::string serial : ramserials)
				{
					sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO RamHwids (UserID, TimeOfCreation, RamSerial, Banned) VALUES (?, ?, ?, ?)");
					statement->setInt(1, userid);
					statement->setString(2, timeofcreation);
					statement->setString(3, serial);
					statement->setInt(4, 0);
					statement->execute();
					delete statement;
				}
			}
		}
		return CreateUserResult::Success;
	}
	catch (sql::SQLException ex)
	{
		std::cout << ex.what() << std::endl; // injection
		return CreateUserResult::Injection;
	}

}


Database::~Database()
{
	Connection->close();
	delete Connection;
}