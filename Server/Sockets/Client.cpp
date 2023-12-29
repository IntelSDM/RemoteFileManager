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