#include "pch.h"
#include "Client.h"


Client::Client(SOCKET socket)
{
	Client::Socket = socket;
	std::thread thread([&] {MessageHandler(); });
	thread.detach();
}
void Client::MessageHandler()
{
	while (true)
	{
		std::string text = Client::ReceiveText();
		json jsoned;
		if (text.size() == 0)
			continue;	
		try
		{
			jsoned = json::parse(text);
		}
		catch (json::exception ex)
		{
			continue;
		}
		if (jsoned.size() == 0)
			continue;
		if (jsoned["ID"] == 0)
		{
			RegisterPacket packet;
			packet.FromJson(jsoned);
			OnRegister(packet);
		}
		if (jsoned["ID"] == 1)
		{
			LoginPacket packet;
			packet.FromJson(jsoned);
			OnLogin(packet);
		}
		if (jsoned["ID"] == 2)
		{
			if (!LoggedIn)
				continue;
			SendFilePacket packet;
			packet.FromJson(jsoned);
			OnFileReceive(packet);
		}
	}
}
void Client::OnFileReceive(SendFilePacket packet)
{
	std::wstring filename(packet.FileName.begin(), packet.FileName.end());
	std::vector<uint8_t> filedata;
	printf("Receiving File: %ls\n", filename.c_str());
	while (true) // recieve the file byte array
	{
		printf("RECING\n");
		std::vector<uint8_t> data = ReceiveData();
		if(data.size() ==0)
			continue;
		else
		{
			filedata = data;
			break;
		}
	}
	if (filedata.size() == 0)
		return;
	DB.StoreFile(Username, filename, filedata);

}
void Client::OnLogin(LoginPacket packet)
{
	std::wstring username(packet.Username.begin(), packet.Username.end());
	std::wstring password(packet.Password.begin(), packet.Password.end());
	auto result = DB.LoginUser(username, password);
	switch (result)
	{
	case LoginUserResult::UserDoesNotExist:
		SendText("User Doesnt Exist");
		break;
	case LoginUserResult::Injection:
		SendText("MYSQL Injection");
		break;
	case LoginUserResult::IncorrectPassword:
		SendText("Incorrect Password");
		break;
	case LoginUserResult::Success:
		SendText("Successful Login");
		LoggedIn = true;
		Username = username;
		break;
	}
}
void Client::OnRegister(RegisterPacket packet)
{
	std::wstring username(packet.Username.begin(), packet.Username.end());
	std::wstring password(packet.Password.begin(), packet.Password.end());
	auto result = DB.CreateUser(username, password);
	switch (result)
	{
	case CreateUserResult::UserAlreadyExists:
		SendText("User Already Exists");
		break;
	case CreateUserResult::Injection:
		SendText("MYSQL Injection");
		break;
	case CreateUserResult::UsernameTooLong:
		SendText("Username Too Long");
		break;
	case CreateUserResult::PasswordTooLong:
		SendText("Password Too Long");
		break;
	case CreateUserResult::UsernameTooShort:
		SendText("Username Too Short");
		break;
	case CreateUserResult::PasswordTooShort:
		SendText("Password Too Short");
		break;
	case CreateUserResult::Success:
		SendText("Successful Register");
		break;
	}

}
void Client::SendText(const std::string& text)
{
	const uint32_t chunksize = BufferSize;
	uint32_t length = static_cast<uint32_t>(text.size());
	uint32_t chunksnum = (length + chunksize - 1) / chunksize;

	send(Client::Socket, reinterpret_cast<const char*>(&length), sizeof(length), 0);

	for (uint32_t i = 0; i < chunksnum; ++i)
	{
		uint32_t offset = i * chunksize;
		uint32_t chunklength = std::min(chunksize, length - offset);

		send(Client::Socket, text.c_str() + offset, chunklength, 0);
	}
}
void Client::SendData(const std::vector<uint8_t>& bytearray)
{
	const uint32_t chunksize = BufferSize;
	uint32_t length = static_cast<uint32_t>(bytearray.size());
	uint32_t chunksnum = (length + chunksize - 1) / chunksize;

	send(Client::Socket, reinterpret_cast<const char*>(&length), sizeof(length), 0);

	for (uint32_t i = 0; i < chunksnum; ++i)
	{
		uint32_t offset = i * chunksize;
		uint32_t chunklength = std::min(chunksize, length - offset);

		send(Client::Socket, reinterpret_cast<const char*>(bytearray.data()) + offset, chunklength, 0);
	}
}
std::vector<uint8_t> Client::ReceiveData()
{
	uint32_t length;
	if (recv(Client::Socket, reinterpret_cast<char*>(&length), sizeof(length), 0) <= 0)
	{
		return {};
	}

	std::vector<uint8_t> data;
	const uint32_t chunksize = BufferSize;
	uint32_t chunksnum = (length + chunksize - 1) / chunksize;

	for (uint32_t i = 0; i < chunksnum; ++i)
	{
		uint32_t offset = i * chunksize;
		uint32_t chunklength = std::min(chunksize, length - offset);

		std::vector<uint8_t> chunk(chunklength);

		int ret = recv(Client::Socket, reinterpret_cast<char*>(chunk.data()), chunklength, 0);

		if (ret <= 0)
		{
			return {};
		}

		data.insert(data.end(), chunk.begin(), chunk.begin() + ret);
	}

	return data;
}
std::string Client::ReceiveText()
{
	uint32_t length;
	if (recv(Client::Socket, reinterpret_cast<char*>(&length), sizeof(length), 0) <= 0)
	{
		return "";
	}

	std::string str;
	const uint32_t chunksize = BufferSize;
	uint32_t chunksnum = (length + chunksize - 1) / chunksize;

	for (uint32_t i = 0; i < chunksnum; ++i)
	{
		uint32_t offset = i * chunksize;
		uint32_t chunklength = std::min(chunksize, length - offset);

		std::vector<uint8_t> bytes(chunklength + 1);
		int ret = recv(Client::Socket, reinterpret_cast<char*>(bytes.data()), chunklength, 0);

		if (ret <= 0)
		{
			return "";
		}

		bytes[ret] = '\0';
		str += reinterpret_cast<char*>(bytes.data());
	}

	return str;
}