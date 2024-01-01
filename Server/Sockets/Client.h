#pragma once
#include "Database.h"
#include "RegisterPacket.h"
#include "LoginPacket.h"
#include "SendFilePacket.h"
class Client
{
protected:
	const int BufferSize = 4096;
	void OnRegister(RegisterPacket packet);
	void OnLogin(LoginPacket packet);
	void OnFileReceive(SendFilePacket packet);
	void OnFileListRequest();
	Database DB;
	bool LoggedIn = false;
	std::wstring Username;
public:
	void SendText(const std::string& Text);
	void SendData(const std::vector<uint8_t>& bytes);
	std::vector<uint8_t> ReceiveData();
	std::string ReceiveText();
	void MessageHandler();
	Client(SOCKET sock);
private:
	SOCKET Socket;

};