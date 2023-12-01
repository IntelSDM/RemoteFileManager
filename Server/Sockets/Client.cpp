#include "pch.h"
#include "Client.h"


Client::Client(SOCKET socket)
{
	Client::Socket = socket;
	std::thread thread([&] {MessageHandler(); });
	thread.detach();
}
constexpr int BufferSize = 4096;
void Client::SendText(std::string text)
{
	text += "|";
	std::vector<uint8_t> plaintext(text.begin(), text.end());
	int32_t Result = send(Client::Socket, (char*)plaintext.data(), (int)plaintext.size(), 0);
}
std::string Client::ReceiveText()
{
	std::vector<uint8_t> 	recbytes;
	uint8_t		buffer[BufferSize];

	while (true)
	{
		int32_t recieved = recv(Client::Socket, (char*)buffer, BufferSize, 0);

		if (recieved < 0)
			break;

		for (int n = 0; n < recieved; ++n)
		{
			recbytes.push_back(buffer[n]);
		}

		if (recieved <= BufferSize)
			break;

	}
	std::string str(recbytes.begin(), recbytes.end());
	return str;
}
void Client::MessageHandler()
{
	while (true)
	{
		std::string message = Client::ReceiveText(); // halts everything here, goes to recieve a message
		if (message.size() == 0)
			return;

		json jsoned = json::parse(message);


	}
}