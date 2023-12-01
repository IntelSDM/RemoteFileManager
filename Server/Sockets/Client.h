#pragma once
class Client
{

public:

	std::string IpAddress;

	void SendText(std::string Text);
	std::string ReceiveText();
	void MessageHandler();
	Client(SOCKET sock);
private:
	SOCKET Socket;

};