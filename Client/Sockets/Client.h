#pragma once
class Client
{
protected:
	const int BufferSize = 4096;
public:

	std::string IpAddress;
	Client(SOCKET socket);
	void SendText(const std::string& Text);
	void SendData(const std::vector<uint8_t>& bytes);
	std::vector<uint8_t> ReceiveData();
	std::string ReceiveText();
private:
	SOCKET Socket;

};