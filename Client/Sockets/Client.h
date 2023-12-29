#pragma once
class Client
{
protected:
	const int BufferSize = 4096;
public:

	std::string IpAddress;
	Client(SOCKET socket);
	void SendText(std::string Text);
	std::string ReceiveText();
private:
	SOCKET Socket;

};