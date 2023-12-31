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
		if (usedtables[Database::ToLower("FilesTable")] == false)
		{
			Connection->createStatement()->execute("CREATE TABLE FilesTable ("
				"FileID INT AUTO_INCREMENT PRIMARY KEY,"
				"UserID INT,"
				"FileName VARCHAR(255),"
				"DateOfCreation VARCHAR(17),"
				"FileData BLOB,"
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

StoreFileResult Database::StoreFile(const std::wstring& username, const std::wstring& filename, const std::vector<uint8_t> filearray)
{
	try 
	{
		auto now = std::chrono::system_clock::now();
		std::time_t timenow = std::chrono::system_clock::to_time_t(now);
		std::tm localtime = *std::localtime(&timenow);

		Connection->setSchema(DatabaseNames[ActiveDatabase]);
		int userid = 0;
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT UserID FROM Users WHERE Username = ?");
			statement->setString(1, ToSQLString(ToLower(username)));
			sql::ResultSet* result = statement->executeQuery();

			if (!result->next())
			{
				std::cout << "User Doesn't Exist" << std::endl;
				return StoreFileResult::UserDoesNotExist;

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
				return StoreFileResult::EncryptionDoesNotExist;
			}
			encryptionkey = result->getString(1);
			encryptionkey = Cryptography.DecryptAES(Cryptography.Base64Decode(encryptionkey), EncryptionKey);
			delete result;
			delete statement;

		}
		{
			std::string year = std::to_string(localtime.tm_year + 1900);
			std::string month = std::to_string(localtime.tm_mon + 1);
			std::string day = std::to_string(localtime.tm_mday);
			std::string hour = std::to_string(localtime.tm_hour);
			std::string minute = std::to_string(localtime.tm_min);
			std::string second = std::to_string(localtime.tm_sec);
			std::string timeofcreation = day + "/" + month + "/" + year + " " + hour + ":" + minute;

			sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO FilesTable (UserID, DateOfCreation, FileName, FileData) VALUES (?, ?, ?, ?)");
			statement->setInt(1, userid);
			std::string str = std::string(filename.begin(), filename.end());
			statement->setString(2, timeofcreation);
			statement->setString(3, Cryptography.Base64Encode(Cryptography.EncryptAES(str, encryptionkey)));
			std::istringstream inputstream(std::string(filearray.begin(), filearray.end()));
			std::istream& stream = inputstream;
			statement->setBlob(4, &stream);
			statement->execute();
			delete statement;

			return StoreFileResult::Success;
		}
	}
	catch (sql::SQLException ex)
	{
	return StoreFileResult::Injection;
	
	}
}
std::vector<int> Database::GetFileIds(const std::wstring& username)
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
				return std::vector<int>();

			}
			userid = result->getInt(1);
			delete result;
			delete statement;
		}
		std::vector<int> fileids;
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT FileID FROM FilesTable WHERE UserID = ?");
			statement->setInt(1, userid);
			sql::ResultSet* result = statement->executeQuery();
			while (result->next())
			{
				fileids.push_back(result->getInt(1));
			}
			delete result;
			delete statement;
		}
		return fileids;
	}
	catch (sql::SQLException ex)
	{
		return std::vector<int>();
	}
}
std::vector<std::string> Database::GetFileNames(const std::wstring& username)
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
				return std::vector<std::string>();

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
				return std::vector<std::string>();
			}
			encryptionkey = result->getString(1);
			encryptionkey = Cryptography.DecryptAES(Cryptography.Base64Decode(encryptionkey), EncryptionKey);
			delete result;
			delete statement;

		}
		std::vector<std::string> filenames;
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT FileName FROM FilesTable WHERE UserID = ?");
			statement->setInt(1, userid);
			sql::ResultSet* result = statement->executeQuery();
			while (result->next())
			{
				filenames.push_back(Cryptography.DecryptAES(Cryptography.Base64Decode(result->getString(1)), encryptionkey));
			}
			delete result;
			delete statement;
		}
		return filenames;
	}
	catch (sql::SQLException ex)
	{
		return std::vector<std::string>();
	}
}
std::vector<uint8_t> Database::GetFile(const std::wstring& username, const int& fileid)
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
				return std::vector<uint8_t>();

			}
			userid = result->getInt(1);
			delete result;
			delete statement;
		}
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT FileData FROM FilesTable WHERE FileID = ? AND UserID = ?");
			statement->setInt(1, fileid);
			statement->setInt(2, userid);
			sql::ResultSet* result = statement->executeQuery();
			if (!result->next())
			{
				std::cout << "File Doesn't Exist" << std::endl;
				return std::vector<uint8_t>();
			}
			std::istream* stream = result->getBlob(1);
			std::vector<uint8_t> filedata;
			while (stream->good())
			{
				filedata.push_back(stream->get());
			}
			delete result;
			delete statement;
			return filedata;
		}
		return std::vector<uint8_t>();
	}
	catch (sql::SQLException ex)
	{
		return std::vector<uint8_t>();
	}
}
std::vector<std::string> Database::GetFileTimes(const std::wstring& username)
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
				return std::vector<std::string>();

			}
			userid = result->getInt(1);
			delete result;
			delete statement;
		}
		std::vector<std::string> filetimes;
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("SELECT DateOfCreation FROM FilesTable WHERE UserID = ?");
			statement->setInt(1, userid);
			sql::ResultSet* result = statement->executeQuery();
			while (result->next())
			{
				filetimes.push_back(result->getString(1));
			}
			delete result;
			delete statement;
		}
		return filetimes;
	}
	catch (sql::SQLException ex)
	{
		return std::vector<std::string>();
	}

}
LoginUserResult Database::LoginUser(const std::wstring& username, const std::wstring& password)
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
		

		return LoginUserResult::Success;
	}
	catch (sql::SQLException ex)
	{
		return LoginUserResult::Injection;
	}

}

CreateUserResult Database::CreateUser(const std::wstring& username, const std::wstring& password)
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
		
		{
			sql::PreparedStatement* statement = Connection->prepareStatement("INSERT INTO Users (Username, Password, Salt) VALUES (?, ?, ?)");
			statement->setString(1, ToSQLString(ToLower(username)));
			statement->setString(2, ToSQLString(sha256(password + salt)));
			statement->setString(3, ToSQLString(salt));
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