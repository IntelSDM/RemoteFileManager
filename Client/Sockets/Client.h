#pragma once
class Client
{

public:

	std::string IpAddress;
	Client(SOCKET socket);
	void SendText(std::string Text);
	std::string ReceiveText();
	void MessageHandler();
private:
	SOCKET Socket;

};