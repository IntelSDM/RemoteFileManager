#pragma once
#include "Database.h"
#include "RegisterPacket.h"
#include "LoginPacket.h"
class Client
{
protected:
	const int BufferSize = 4096;
	void OnRegister(RegisterPacket packet);
	void OnLogin(LoginPacket packet);
	Database DB;
	bool LoggedIn = false;
public:
	void SendText(std::string Text);
	std::string ReceiveText();
	void MessageHandler();
	Client(SOCKET sock);
private:
	SOCKET Socket;

};