#include "Pch.h"
#include "Sockets.h"
#include "Client.h"
sockaddr_in HInt;
SOCKET Listening;
sockaddr_in ClientAddress;
void CreateServer()
{
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsData); // start the server

    Listening = socket(AF_INET, SOCK_STREAM, 0); // create socket instance

    HInt.sin_family = AF_INET; // declare the ip and port and connection rules
    HInt.sin_port = htons(51000);
    HInt.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(Listening, reinterpret_cast<sockaddr*>(&HInt), sizeof(HInt)); // bind the connection rules to the listening socket
    listen(Listening, SOMAXCONN); // keep socket open
}

void AcceptClients()
{
    std::cout << "connection start\n";
    while (true)
    {

        int size = sizeof(ClientAddress);
        SOCKET clientsocket = accept(Listening, reinterpret_cast<SOCKADDR*>(&ClientAddress), &size);
        if (clientsocket != INVALID_SOCKET)
        {
            std::cout << "connection\n";
            Client* client = new Client(clientsocket);
        }

    }
}