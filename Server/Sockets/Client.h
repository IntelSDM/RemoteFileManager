#pragma once
#include "Database.h"
#include "RegisterPacket.h"
class Client
{
protected:
	const int BufferSize = 4096;
	void OnRegister(RegisterPacket packet);
	Database DB;
public:
	void SendText(std::string Text);
	std::string ReceiveText();
	void MessageHandler();
	Client(SOCKET sock);
private:
	SOCKET Socket;

};